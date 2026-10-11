#pragma once

#include "repository/IProductRepository.h"
#include "repository/PostgresConnection.h"

namespace sudharshan::sudharshanmart {

class PostgresProductRepository : public IProductRepository {
public:
    explicit PostgresProductRepository(PostgresConnection& db_conn) : db_conn_(db_conn) {}

    std::optional<Product> FindById(int64_t id) override;
    std::vector<Product> FindAll(const std::string& query = "", const std::string& category = "") override;
    std::vector<Product> FindBySellerId(int64_t seller_id) override;
    Product CreateProduct(const Product& product) override;
    Product UpdateProduct(const Product& product) override;
    bool DeleteProduct(int64_t id) override;

private:
    PostgresConnection& db_conn_;
};

} // namespace sudharshan::sudharshanmart
