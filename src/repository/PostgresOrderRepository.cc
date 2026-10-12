#include "repository/PostgresOrderRepository.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>
#include <cstring>

namespace srikanth::srikanthmart {

namespace {
constexpr const char* kOrderColumns =
    "id, buyer_id, status, total_amount_cents, "
    "to_char(created_at, 'YYYY-MM-DD HH24:MI:SS') AS created_at";

Order ReadOrder(PGresult* res, int row) {
    Order o;
    o.id = PgInt64(res, row, 0);
    o.buyer_id = PgInt64(res, row, 1);
    o.status = StringToOrderStatus(PgText(res, row, 2));
    o.total_amount = Money(PgInt64(res, row, 3));
    o.created_at = PgText(res, row, 4);
    return o;
}

bool CmdAffectedZero(PGresult* res) {
    const char* tuples = PQcmdTuples(res);
    return tuples == nullptr || std::strlen(tuples) == 0 || std::strcmp(tuples, "0") == 0;
}
} // namespace

Order PostgresOrderRepository::CreateOrder(int64_t buyer_id, const std::vector<CartItem>& items, int64_t total_amount_cents) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgTransaction tx(conn);

    PgResult order_res = PgExecParams(conn,
        "INSERT INTO orders (buyer_id, status, total_amount_cents) VALUES ($1, 'PENDING', $2) RETURNING id;",
        { std::to_string(buyer_id), std::to_string(total_amount_cents) });
    if (!order_res.IsOk()) {
        throw InternalServerException("Failed to execute order creation");
    }
    int64_t order_id = PgInt64(order_res.Get(), 0, 0);

    for (const auto& item : items) {
        PgResult item_res = PgExecParams(conn,
            "INSERT INTO order_items (order_id, product_id, quantity, unit_price_cents) VALUES ($1, $2, $3, $4);",
            { std::to_string(order_id), std::to_string(item.product_id),
              std::to_string(item.quantity), std::to_string(item.unit_price.GetCents()) });
        if (!item_res.IsOk()) {
            throw InternalServerException("Failed to insert order line item");
        }

        PgResult stock_res = PgExecParams(conn,
            "UPDATE products SET stock_qty = stock_qty - $1 WHERE id = $2 AND stock_qty >= $1;",
            { std::to_string(item.quantity), std::to_string(item.product_id) });
        if (!stock_res.IsOk() || CmdAffectedZero(stock_res.Get())) {
            throw ConflictException("Insufficient product stock for product: " + item.product_name);
        }
    }

    tx.Commit();

    Order order;
    order.id = order_id;
    order.buyer_id = buyer_id;
    order.status = OrderStatus::kPending;
    order.total_amount = Money(total_amount_cents);
    return order;
}

std::optional<Order> PostgresOrderRepository::FindById(int64_t order_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        std::string("SELECT ") + kOrderColumns + " FROM orders WHERE id = $1;",
        { std::to_string(order_id) });
    if (!res.IsOk()) {
        throw InternalServerException("Failed to prepare find order statement");
    }
    if (PQntuples(res.Get()) > 0) {
        Order o = ReadOrder(res.Get(), 0);
        PopulateOrderItems(o);
        return o;
    }
    return std::nullopt;
}

std::vector<Order> PostgresOrderRepository::FindByBuyerId(int64_t buyer_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        std::string("SELECT ") + kOrderColumns + " FROM orders WHERE buyer_id = $1 ORDER BY id DESC;",
        { std::to_string(buyer_id) });
    if (!res.IsOk()) {
        throw InternalServerException("Failed to prepare buyer orders statement");
    }

    std::vector<Order> list;
    const int rows = PQntuples(res.Get());
    for (int i = 0; i < rows; ++i) {
        Order o = ReadOrder(res.Get(), i);
        PopulateOrderItems(o);
        list.push_back(o);
    }
    return list;
}

std::vector<Order> PostgresOrderRepository::FindIncomingForSeller(int64_t seller_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "SELECT DISTINCT o.id, o.buyer_id, o.status, o.total_amount_cents, "
        "to_char(o.created_at, 'YYYY-MM-DD HH24:MI:SS') AS created_at "
        "FROM orders o "
        "JOIN order_items oi ON o.id = oi.order_id "
        "JOIN products p ON oi.product_id = p.id "
        "WHERE p.seller_id = $1 ORDER BY o.id DESC;",
        { std::to_string(seller_id) });
    if (!res.IsOk()) {
        throw InternalServerException("Failed to prepare seller orders statement");
    }

    std::vector<Order> list;
    const int rows = PQntuples(res.Get());
    for (int i = 0; i < rows; ++i) {
        Order o = ReadOrder(res.Get(), i);
        PopulateOrderItems(o);
        list.push_back(o);
    }
    return list;
}

std::vector<Order> PostgresOrderRepository::FindAll() {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        std::string("SELECT ") + kOrderColumns + " FROM orders ORDER BY id DESC;", {});
    if (!res.IsOk()) {
        throw InternalServerException("Failed to prepare all orders statement");
    }

    std::vector<Order> list;
    const int rows = PQntuples(res.Get());
    for (int i = 0; i < rows; ++i) {
        Order o = ReadOrder(res.Get(), i);
        PopulateOrderItems(o);
        list.push_back(o);
    }
    return list;
}

bool PostgresOrderRepository::UpdateOrderStatus(int64_t order_id, OrderStatus status) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "UPDATE orders SET status = $1 WHERE id = $2;",
        { OrderStatusToString(status), std::to_string(order_id) });
    return res.IsOk();
}

int64_t PostgresOrderRepository::GetSellerRevenue(int64_t seller_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "SELECT COALESCE(SUM(oi.quantity * oi.unit_price_cents), 0) "
        "FROM order_items oi "
        "JOIN products p ON oi.product_id = p.id "
        "JOIN orders o ON oi.order_id = o.id "
        "WHERE p.seller_id = $1 AND o.status != 'CANCELLED';",
        { std::to_string(seller_id) });
    if (res.IsOk() && PQntuples(res.Get()) > 0) {
        return PgInt64(res.Get(), 0, 0);
    }
    return 0;
}

int64_t PostgresOrderRepository::GetSellerOrderCount(int64_t seller_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "SELECT COUNT(DISTINCT o.id) "
        "FROM orders o "
        "JOIN order_items oi ON o.id = oi.order_id "
        "JOIN products p ON oi.product_id = p.id "
        "WHERE p.seller_id = $1;",
        { std::to_string(seller_id) });
    if (res.IsOk() && PQntuples(res.Get()) > 0) {
        return PgInt64(res.Get(), 0, 0);
    }
    return 0;
}

void PostgresOrderRepository::PopulateOrderItems(Order& order) {
    PGconn* conn = db_conn_.Get();
    PgResult res = PgExecParams(conn,
        "SELECT oi.id, oi.order_id, oi.product_id, oi.quantity, oi.unit_price_cents, "
        "to_char(oi.created_at, 'YYYY-MM-DD HH24:MI:SS') AS created_at, "
        "p.name, p.seller_id "
        "FROM order_items oi "
        "JOIN products p ON oi.product_id = p.id "
        "WHERE oi.order_id = $1 ORDER BY oi.id ASC;",
        { std::to_string(order.id) });
    if (!res.IsOk()) {
        return;
    }

    const int rows = PQntuples(res.Get());
    for (int i = 0; i < rows; ++i) {
        OrderItem oi;
        oi.id = PgInt64(res.Get(), i, 0);
        oi.order_id = PgInt64(res.Get(), i, 1);
        oi.product_id = PgInt64(res.Get(), i, 2);
        oi.quantity = PgInt32(res.Get(), i, 3);
        oi.unit_price = Money(PgInt64(res.Get(), i, 4));
        oi.created_at = PgText(res.Get(), i, 5);
        oi.product_name = PgText(res.Get(), i, 6);
        oi.seller_id = PgInt64(res.Get(), i, 7);
        order.items.push_back(oi);
    }
}

} // namespace srikanth::srikanthmart
