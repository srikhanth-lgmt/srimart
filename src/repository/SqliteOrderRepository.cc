#include "repository/SqliteOrderRepository.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

Order SqliteOrderRepository::CreateOrder(int64_t buyer_id, const std::vector<CartItem>& items, int64_t total_amount_cents) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    // Begin RAII transaction
    SqliteTransaction tx(db);

    const char* order_sql = "INSERT INTO orders (buyer_id, status, total_amount_cents) VALUES (?, 'PENDING', ?);";
    sqlite3_stmt* order_stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, order_sql, -1, &order_stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException("Failed to prepare order creation statement");
    }
    StatementRAII order_stmt(order_stmt_raw);
    sqlite3_bind_int64(order_stmt.Get(), 1, buyer_id);
    sqlite3_bind_int64(order_stmt.Get(), 2, total_amount_cents);

    if (sqlite3_step(order_stmt.Get()) != SQLITE_DONE) {
        throw InternalServerException("Failed to execute order creation");
    }

    int64_t order_id = sqlite3_last_insert_rowid(db);

    const char* item_sql = "INSERT INTO order_items (order_id, product_id, quantity, unit_price_cents) VALUES (?, ?, ?, ?);";
    const char* stock_sql = "UPDATE products SET stock_qty = stock_qty - ? WHERE id = ? AND stock_qty >= ?;";

    for (const auto& item : items) {
        // Insert order line item
        sqlite3_stmt* item_stmt_raw = nullptr;
        if (sqlite3_prepare_v2(db, item_sql, -1, &item_stmt_raw, nullptr) != SQLITE_OK) {
            throw InternalServerException("Failed to prepare order item statement");
        }
        StatementRAII item_stmt(item_stmt_raw);
        sqlite3_bind_int64(item_stmt.Get(), 1, order_id);
        sqlite3_bind_int64(item_stmt.Get(), 2, item.product_id);
        sqlite3_bind_int(item_stmt.Get(), 3, item.quantity);
        sqlite3_bind_int64(item_stmt.Get(), 4, item.unit_price.GetCents());
        if (sqlite3_step(item_stmt.Get()) != SQLITE_DONE) {
            throw InternalServerException("Failed to insert order line item");
        }

        // Atomically decrement stock
        sqlite3_stmt* stock_stmt_raw = nullptr;
        if (sqlite3_prepare_v2(db, stock_sql, -1, &stock_stmt_raw, nullptr) != SQLITE_OK) {
            throw InternalServerException("Failed to prepare stock update statement");
        }
        StatementRAII stock_stmt(stock_stmt_raw);
        sqlite3_bind_int(stock_stmt.Get(), 1, item.quantity);
        sqlite3_bind_int64(stock_stmt.Get(), 2, item.product_id);
        sqlite3_bind_int(stock_stmt.Get(), 3, item.quantity);
        if (sqlite3_step(stock_stmt.Get()) != SQLITE_DONE) {
            throw ConflictException("Insufficient product stock for product: " + item.product_name);
        }
    }

    // Commit transaction
    tx.Commit();

    Order order;
    order.id = order_id;
    order.buyer_id = buyer_id;
    order.status = OrderStatus::kPending;
    order.total_amount = Money(total_amount_cents);
    return order;
}

std::optional<Order> SqliteOrderRepository::FindById(int64_t order_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT id, buyer_id, status, total_amount_cents, created_at FROM orders WHERE id = ?;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException("Failed to prepare find order statement");
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, order_id);

    if (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        Order o;
        o.id = sqlite3_column_int64(stmt.Get(), 0);
        o.buyer_id = sqlite3_column_int64(stmt.Get(), 1);
        o.status = StringToOrderStatus(reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 2)));
        o.total_amount = Money(sqlite3_column_int64(stmt.Get(), 3));
        o.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 4));
        PopulateOrderItems(o);
        return o;
    }
    return std::nullopt;
}

std::vector<Order> SqliteOrderRepository::FindByBuyerId(int64_t buyer_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT id, buyer_id, status, total_amount_cents, created_at FROM orders WHERE buyer_id = ? ORDER BY id DESC;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException("Failed to prepare buyer orders statement");
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, buyer_id);

    std::vector<Order> list;
    while (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        Order o;
        o.id = sqlite3_column_int64(stmt.Get(), 0);
        o.buyer_id = sqlite3_column_int64(stmt.Get(), 1);
        o.status = StringToOrderStatus(reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 2)));
        o.total_amount = Money(sqlite3_column_int64(stmt.Get(), 3));
        o.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 4));
        PopulateOrderItems(o);
        list.push_back(o);
    }
    return list;
}

std::vector<Order> SqliteOrderRepository::FindIncomingForSeller(int64_t seller_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT DISTINCT o.id, o.buyer_id, o.status, o.total_amount_cents, o.created_at "
                      "FROM orders o "
                      "JOIN order_items oi ON o.id = oi.order_id "
                      "JOIN products p ON oi.product_id = p.id "
                      "WHERE p.seller_id = ? ORDER BY o.id DESC;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException("Failed to prepare seller orders statement");
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, seller_id);

    std::vector<Order> list;
    while (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        Order o;
        o.id = sqlite3_column_int64(stmt.Get(), 0);
        o.buyer_id = sqlite3_column_int64(stmt.Get(), 1);
        o.status = StringToOrderStatus(reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 2)));
        o.total_amount = Money(sqlite3_column_int64(stmt.Get(), 3));
        o.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 4));
        PopulateOrderItems(o);
        list.push_back(o);
    }
    return list;
}

std::vector<Order> SqliteOrderRepository::FindAll() {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT id, buyer_id, status, total_amount_cents, created_at FROM orders ORDER BY id DESC;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException("Failed to prepare all orders statement");
    }
    StatementRAII stmt(stmt_raw);

    std::vector<Order> list;
    while (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        Order o;
        o.id = sqlite3_column_int64(stmt.Get(), 0);
        o.buyer_id = sqlite3_column_int64(stmt.Get(), 1);
        o.status = StringToOrderStatus(reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 2)));
        o.total_amount = Money(sqlite3_column_int64(stmt.Get(), 3));
        o.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 4));
        PopulateOrderItems(o);
        list.push_back(o);
    }
    return list;
}

bool SqliteOrderRepository::UpdateOrderStatus(int64_t order_id, OrderStatus status) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "UPDATE orders SET status = ? WHERE id = ?;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException("Failed to prepare status update statement");
    }
    StatementRAII stmt(stmt_raw);
    std::string status_str = OrderStatusToString(status);
    sqlite3_bind_text(stmt.Get(), 1, status_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt.Get(), 2, order_id);

    return sqlite3_step(stmt.Get()) == SQLITE_DONE;
}

int64_t SqliteOrderRepository::GetSellerRevenue(int64_t seller_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT COALESCE(SUM(oi.quantity * oi.unit_price_cents), 0) "
                      "FROM order_items oi "
                      "JOIN products p ON oi.product_id = p.id "
                      "JOIN orders o ON oi.order_id = o.id "
                      "WHERE p.seller_id = ? AND o.status != 'CANCELLED';";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        return 0;
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, seller_id);
    if (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        return sqlite3_column_int64(stmt.Get(), 0);
    }
    return 0;
}

int64_t SqliteOrderRepository::GetSellerOrderCount(int64_t seller_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT COUNT(DISTINCT o.id) "
                      "FROM orders o "
                      "JOIN order_items oi ON o.id = oi.order_id "
                      "JOIN products p ON oi.product_id = p.id "
                      "WHERE p.seller_id = ?;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        return 0;
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, seller_id);
    if (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        return sqlite3_column_int64(stmt.Get(), 0);
    }
    return 0;
}

void SqliteOrderRepository::PopulateOrderItems(Order& order) {
    sqlite3* db = db_conn_.GetDb();
    const char* sql = "SELECT oi.id, oi.order_id, oi.product_id, oi.quantity, oi.unit_price_cents, oi.created_at, "
                      "p.name, p.seller_id "
                      "FROM order_items oi "
                      "JOIN products p ON oi.product_id = p.id "
                      "WHERE oi.order_id = ?;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        return;
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, order.id);

    while (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        OrderItem oi;
        oi.id = sqlite3_column_int64(stmt.Get(), 0);
        oi.order_id = sqlite3_column_int64(stmt.Get(), 1);
        oi.product_id = sqlite3_column_int64(stmt.Get(), 2);
        oi.quantity = sqlite3_column_int(stmt.Get(), 3);
        oi.unit_price = Money(sqlite3_column_int64(stmt.Get(), 4));
        oi.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 5));
        oi.product_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 6));
        oi.seller_id = sqlite3_column_int64(stmt.Get(), 7);
        order.items.push_back(oi);
    }
}

} // namespace srikanth::srikanthmart
