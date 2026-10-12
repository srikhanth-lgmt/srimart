#include "repository/PostgresCartRepository.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

std::vector<CartItem> PostgresCartRepository::GetCart(int64_t user_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "SELECT c.id, c.user_id, c.product_id, c.quantity, "
        "to_char(c.created_at, 'YYYY-MM-DD HH24:MI:SS') AS created_at, "
        "p.name, p.price_cents, p.image_url, p.category "
        "FROM cart_items c "
        "JOIN products p ON c.product_id = p.id "
        "WHERE c.user_id = $1 "
        "ORDER BY c.id ASC;",
        { std::to_string(user_id) });
    if (!res.IsOk()) {
        throw InternalServerException(std::string("Failed to query cart: ") + PQerrorMessage(conn));
    }

    std::vector<CartItem> items;
    const int rows = PQntuples(res.Get());
    for (int i = 0; i < rows; ++i) {
        CartItem ci;
        ci.id = PgInt64(res.Get(), i, 0);
        ci.user_id = PgInt64(res.Get(), i, 1);
        ci.product_id = PgInt64(res.Get(), i, 2);
        ci.quantity = PgInt32(res.Get(), i, 3);
        ci.created_at = PgText(res.Get(), i, 4);
        ci.product_name = PgText(res.Get(), i, 5);
        ci.unit_price = Money(PgInt64(res.Get(), i, 6));
        ci.product_image_url = PgText(res.Get(), i, 7);
        ci.product_category = PgText(res.Get(), i, 8);
        items.push_back(ci);
    }
    return items;
}

CartItem PostgresCartRepository::AddOrUpdateItem(int64_t user_id, int64_t product_id, int32_t quantity) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    // Check if item already exists in the user's cart.
    PgResult check = PgExecParams(conn,
        "SELECT id, quantity FROM cart_items WHERE user_id = $1 AND product_id = $2;",
        { std::to_string(user_id), std::to_string(product_id) });
    if (!check.IsOk()) {
        throw InternalServerException("Failed to check cart item");
    }

    if (PQntuples(check.Get()) > 0) {
        int64_t cart_id = PgInt64(check.Get(), 0, 0);
        int32_t current_qty = PgInt32(check.Get(), 0, 1);
        int32_t new_qty = current_qty + quantity;

        PgResult update = PgExecParams(conn,
            "UPDATE cart_items SET quantity = $1 WHERE id = $2;",
            { std::to_string(new_qty), std::to_string(cart_id) });
        if (!update.IsOk()) {
            throw InternalServerException("Failed to update cart item");
        }

        CartItem item;
        item.id = cart_id;
        item.user_id = user_id;
        item.product_id = product_id;
        item.quantity = new_qty;
        return item;
    }

    PgResult insert = PgExecParams(conn,
        "INSERT INTO cart_items (user_id, product_id, quantity) VALUES ($1, $2, $3) RETURNING id;",
        { std::to_string(user_id), std::to_string(product_id), std::to_string(quantity) });
    if (!insert.IsOk()) {
        throw InternalServerException("Failed to insert item into cart");
    }

    CartItem item;
    item.id = PgInt64(insert.Get(), 0, 0);
    item.user_id = user_id;
    item.product_id = product_id;
    item.quantity = quantity;
    return item;
}

bool PostgresCartRepository::RemoveItem(int64_t user_id, int64_t product_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "DELETE FROM cart_items WHERE user_id = $1 AND product_id = $2;",
        { std::to_string(user_id), std::to_string(product_id) });
    return res.IsOk();
}

void PostgresCartRepository::ClearCart(int64_t user_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "DELETE FROM cart_items WHERE user_id = $1;",
        { std::to_string(user_id) });
    if (!res.IsOk()) {
        throw InternalServerException("Failed to clear cart");
    }
}

} // namespace srikanth::srikanthmart
