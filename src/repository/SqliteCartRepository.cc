#include "repository/SqliteCartRepository.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

std::vector<CartItem> SqliteCartRepository::GetCart(int64_t user_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT c.id, c.user_id, c.product_id, c.quantity, c.created_at, "
                      "p.name, p.price_cents, p.image_url, p.category "
                      "FROM cart_items c "
                      "JOIN products p ON c.product_id = p.id "
                      "WHERE c.user_id = ? "
                      "ORDER BY c.id ASC;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, user_id);

    std::vector<CartItem> items;
    while (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        CartItem ci;
        ci.id = sqlite3_column_int64(stmt.Get(), 0);
        ci.user_id = sqlite3_column_int64(stmt.Get(), 1);
        ci.product_id = sqlite3_column_int64(stmt.Get(), 2);
        ci.quantity = sqlite3_column_int(stmt.Get(), 3);
        ci.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 4));
        ci.product_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 5));
        ci.unit_price = Money(sqlite3_column_int64(stmt.Get(), 6));
        const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 7));
        ci.product_image_url = img ? img : "";
        ci.product_category = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 8));
        items.push_back(ci);
    }
    return items;
}

CartItem SqliteCartRepository::AddOrUpdateItem(int64_t user_id, int64_t product_id, int32_t quantity) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    // Check if item exists in user cart
    const char* check_sql = "SELECT id, quantity FROM cart_items WHERE user_id = ? AND product_id = ?;";
    sqlite3_stmt* check_stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, check_sql, -1, &check_stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException("Failed to prepare cart check statement");
    }
    StatementRAII check_stmt(check_stmt_raw);
    sqlite3_bind_int64(check_stmt.Get(), 1, user_id);
    sqlite3_bind_int64(check_stmt.Get(), 2, product_id);

    if (sqlite3_step(check_stmt.Get()) == SQLITE_ROW) {
        int64_t cart_id = sqlite3_column_int64(check_stmt.Get(), 0);
        int32_t current_qty = sqlite3_column_int(check_stmt.Get(), 1);
        int32_t new_qty = current_qty + quantity;

        const char* update_sql = "UPDATE cart_items SET quantity = ? WHERE id = ?;";
        sqlite3_stmt* update_stmt_raw = nullptr;
        if (sqlite3_prepare_v2(db, update_sql, -1, &update_stmt_raw, nullptr) != SQLITE_OK) {
            throw InternalServerException("Failed to prepare cart update statement");
        }
        StatementRAII update_stmt(update_stmt_raw);
        sqlite3_bind_int(update_stmt.Get(), 1, new_qty);
        sqlite3_bind_int64(update_stmt.Get(), 2, cart_id);
        sqlite3_step(update_stmt.Get());

        CartItem item;
        item.id = cart_id;
        item.user_id = user_id;
        item.product_id = product_id;
        item.quantity = new_qty;
        return item;
    } else {
        const char* insert_sql = "INSERT INTO cart_items (user_id, product_id, quantity) VALUES (?, ?, ?);";
        sqlite3_stmt* insert_stmt_raw = nullptr;
        if (sqlite3_prepare_v2(db, insert_sql, -1, &insert_stmt_raw, nullptr) != SQLITE_OK) {
            throw InternalServerException("Failed to prepare cart insert statement");
        }
        StatementRAII insert_stmt(insert_stmt_raw);
        sqlite3_bind_int64(insert_stmt.Get(), 1, user_id);
        sqlite3_bind_int64(insert_stmt.Get(), 2, product_id);
        sqlite3_bind_int(insert_stmt.Get(), 3, quantity);

        if (sqlite3_step(insert_stmt.Get()) != SQLITE_DONE) {
            throw InternalServerException("Failed to insert item into cart");
        }

        CartItem item;
        item.id = sqlite3_last_insert_rowid(db);
        item.user_id = user_id;
        item.product_id = product_id;
        item.quantity = quantity;
        return item;
    }
}

bool SqliteCartRepository::RemoveItem(int64_t user_id, int64_t product_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "DELETE FROM cart_items WHERE user_id = ? AND product_id = ?;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException("Failed to prepare cart remove statement");
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, user_id);
    sqlite3_bind_int64(stmt.Get(), 2, product_id);

    return sqlite3_step(stmt.Get()) == SQLITE_DONE;
}

void SqliteCartRepository::ClearCart(int64_t user_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "DELETE FROM cart_items WHERE user_id = ?;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException("Failed to prepare clear cart statement");
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, user_id);
    sqlite3_step(stmt.Get());
}

} // namespace srikanth::srikanthmart
