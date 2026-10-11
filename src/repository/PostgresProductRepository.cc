#include "repository/PostgresProductRepository.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace sudharshan::sudharshanmart {

namespace {
constexpr const char* kSelectColumns =
    "id, seller_id, name, description, price_cents, stock_qty, category, image_url, "
    "to_char(created_at, 'YYYY-MM-DD HH24:MI:SS') AS created_at";

Product ReadProduct(PGresult* res, int row) {
    Product p;
    p.id = PgInt64(res, row, 0);
    p.seller_id = PgInt64(res, row, 1);
    p.name = PgText(res, row, 2);
    p.description = PgText(res, row, 3);
    p.price = Money(PgInt64(res, row, 4));
    p.stock_qty = PgInt32(res, row, 5);
    p.category = PgText(res, row, 6);
    p.image_url = PgText(res, row, 7);
    p.created_at = PgText(res, row, 8);
    return p;
}
} // namespace

std::optional<Product> PostgresProductRepository::FindById(int64_t id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        std::string("SELECT ") + kSelectColumns + " FROM products WHERE id = $1;",
        { std::to_string(id) });
    if (!res.IsOk()) {
        throw InternalServerException(std::string("Failed to query product: ") + PQerrorMessage(conn));
    }
    if (PQntuples(res.Get()) > 0) {
        return ReadProduct(res.Get(), 0);
    }
    return std::nullopt;
}

std::vector<Product> PostgresProductRepository::FindAll(const std::string& query, const std::string& category) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    std::string sql = std::string("SELECT ") + kSelectColumns + " FROM products WHERE 1=1 ";
    std::vector<std::optional<std::string>> params;
    int idx = 1;
    if (!query.empty()) {
        const std::string placeholder = "$" + std::to_string(idx++);
        sql += "AND (name ILIKE " + placeholder + " OR description ILIKE " + placeholder + ") ";
        params.push_back("%" + query + "%");
    }
    if (!category.empty()) {
        sql += "AND category = $" + std::to_string(idx++) + " ";
        params.push_back(category);
    }
    sql += "ORDER BY id DESC;";

    PgResult res = PgExecParams(conn, sql, params);
    if (!res.IsOk()) {
        throw InternalServerException(std::string("Failed to query products: ") + PQerrorMessage(conn));
    }

    std::vector<Product> products;
    const int rows = PQntuples(res.Get());
    for (int i = 0; i < rows; ++i) {
        products.push_back(ReadProduct(res.Get(), i));
    }
    return products;
}

std::vector<Product> PostgresProductRepository::FindBySellerId(int64_t seller_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        std::string("SELECT ") + kSelectColumns + " FROM products WHERE seller_id = $1 ORDER BY id DESC;",
        { std::to_string(seller_id) });
    if (!res.IsOk()) {
        throw InternalServerException(std::string("Failed to query seller products: ") + PQerrorMessage(conn));
    }

    std::vector<Product> products;
    const int rows = PQntuples(res.Get());
    for (int i = 0; i < rows; ++i) {
        products.push_back(ReadProduct(res.Get(), i));
    }
    return products;
}

Product PostgresProductRepository::CreateProduct(const Product& product) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "INSERT INTO products (seller_id, name, description, price_cents, stock_qty, category, image_url) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7) RETURNING id;",
        { std::to_string(product.seller_id), product.name, product.description,
          std::to_string(product.price.GetCents()), std::to_string(product.stock_qty),
          product.category, product.image_url });
    if (!res.IsOk()) {
        throw InternalServerException(std::string("Failed to insert product: ") + PQerrorMessage(conn));
    }

    Product created = product;
    created.id = PgInt64(res.Get(), 0, 0);
    return created;
}

Product PostgresProductRepository::UpdateProduct(const Product& product) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "UPDATE products SET name = $1, description = $2, price_cents = $3, stock_qty = $4, "
        "category = $5, image_url = $6 WHERE id = $7 AND seller_id = $8;",
        { product.name, product.description, std::to_string(product.price.GetCents()),
          std::to_string(product.stock_qty), product.category, product.image_url,
          std::to_string(product.id), std::to_string(product.seller_id) });
    if (!res.IsOk()) {
        throw InternalServerException(std::string("Failed to update product: ") + PQerrorMessage(conn));
    }
    return product;
}

bool PostgresProductRepository::DeleteProduct(int64_t id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "DELETE FROM products WHERE id = $1;",
        { std::to_string(id) });
    return res.IsOk();
}

} // namespace sudharshan::sudharshanmart
