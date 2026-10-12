#include "repository/SqliteProductRepository.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

std::optional<Product> SqliteProductRepository::FindById(int64_t id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT id, seller_id, name, description, price_cents, stock_qty, category, image_url, created_at "
                      "FROM products WHERE id = ?;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, id);

    if (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        Product p;
        p.id = sqlite3_column_int64(stmt.Get(), 0);
        p.seller_id = sqlite3_column_int64(stmt.Get(), 1);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 2));
        p.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 3));
        p.price = Money(sqlite3_column_int64(stmt.Get(), 4));
        p.stock_qty = sqlite3_column_int(stmt.Get(), 5);
        p.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 6));
        const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 7));
        p.image_url = img ? img : "";
        p.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 8));
        return p;
    }
    return std::nullopt;
}

std::vector<Product> SqliteProductRepository::FindAll(const std::string& query, const std::string& category) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    std::string sql = "SELECT id, seller_id, name, description, price_cents, stock_qty, category, image_url, created_at "
                      "FROM products WHERE 1=1 ";
    if (!query.empty()) {
        sql += "AND (name LIKE ? OR description LIKE ?) ";
    }
    if (!category.empty()) {
        sql += "AND category = ? ";
    }
    sql += "ORDER BY id DESC;";

    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }
    StatementRAII stmt(stmt_raw);

    int param_idx = 1;
    std::string wild_query = "%" + query + "%";
    if (!query.empty()) {
        sqlite3_bind_text(stmt.Get(), param_idx++, wild_query.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt.Get(), param_idx++, wild_query.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!category.empty()) {
        sqlite3_bind_text(stmt.Get(), param_idx++, category.c_str(), -1, SQLITE_TRANSIENT);
    }

    std::vector<Product> products;
    while (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        Product p;
        p.id = sqlite3_column_int64(stmt.Get(), 0);
        p.seller_id = sqlite3_column_int64(stmt.Get(), 1);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 2));
        p.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 3));
        p.price = Money(sqlite3_column_int64(stmt.Get(), 4));
        p.stock_qty = sqlite3_column_int(stmt.Get(), 5);
        p.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 6));
        const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 7));
        p.image_url = img ? img : "";
        p.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 8));
        products.push_back(p);
    }
    return products;
}

std::vector<Product> SqliteProductRepository::FindBySellerId(int64_t seller_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT id, seller_id, name, description, price_cents, stock_qty, category, image_url, created_at "
                      "FROM products WHERE seller_id = ? ORDER BY id DESC;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, seller_id);

    std::vector<Product> products;
    while (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        Product p;
        p.id = sqlite3_column_int64(stmt.Get(), 0);
        p.seller_id = sqlite3_column_int64(stmt.Get(), 1);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 2));
        p.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 3));
        p.price = Money(sqlite3_column_int64(stmt.Get(), 4));
        p.stock_qty = sqlite3_column_int(stmt.Get(), 5);
        p.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 6));
        const char* img = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 7));
        p.image_url = img ? img : "";
        p.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 8));
        products.push_back(p);
    }
    return products;
}

Product SqliteProductRepository::CreateProduct(const Product& product) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "INSERT INTO products (seller_id, name, description, price_cents, stock_qty, category, image_url) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }
    StatementRAII stmt(stmt_raw);

    sqlite3_bind_int64(stmt.Get(), 1, product.seller_id);
    sqlite3_bind_text(stmt.Get(), 2, product.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.Get(), 3, product.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt.Get(), 4, product.price.GetCents());
    sqlite3_bind_int(stmt.Get(), 5, product.stock_qty);
    sqlite3_bind_text(stmt.Get(), 6, product.category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.Get(), 7, product.image_url.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt.Get()) != SQLITE_DONE) {
        throw InternalServerException(std::string("Failed to insert product: ") + sqlite3_errmsg(db));
    }

    Product created = product;
    created.id = sqlite3_last_insert_rowid(db);
    return created;
}

Product SqliteProductRepository::UpdateProduct(const Product& product) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "UPDATE products SET name = ?, description = ?, price_cents = ?, stock_qty = ?, category = ?, image_url = ? "
                      "WHERE id = ? AND seller_id = ?;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }
    StatementRAII stmt(stmt_raw);

    sqlite3_bind_text(stmt.Get(), 1, product.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.Get(), 2, product.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt.Get(), 3, product.price.GetCents());
    sqlite3_bind_int(stmt.Get(), 4, product.stock_qty);
    sqlite3_bind_text(stmt.Get(), 6, product.image_url.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt.Get(), 7, product.id);
    sqlite3_bind_int64(stmt.Get(), 8, product.seller_id);

    if (sqlite3_step(stmt.Get()) != SQLITE_DONE) {
        throw InternalServerException(std::string("Failed to update product: ") + sqlite3_errmsg(db));
    }
    return product;
}

bool SqliteProductRepository::DeleteProduct(int64_t id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "DELETE FROM products WHERE id = ?;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, id);

    return sqlite3_step(stmt.Get()) == SQLITE_DONE;
}

} // namespace srikanth::srikanthmart
