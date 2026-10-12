#pragma once

#include <optional>
#include <vector>
#include <string>
#include <memory>
#include "model/Product.h"

namespace srikanth::srikanthmart {

/**
 * @brief Abstract repository interface for Product data access.
 */
class IProductRepository {
public:
    virtual ~IProductRepository() = default;

    /**
     * @brief Find product by primary key ID.
     */
    virtual std::optional<Product> FindById(int64_t id) = 0;

    /**
     * @brief Search and filter products by search query and optional category.
     */
    virtual std::vector<Product> FindAll(const std::string& query = "", const std::string& category = "") = 0;

    /**
     * @brief Find products owned by a specific seller.
     */
    virtual std::vector<Product> FindBySellerId(int64_t seller_id) = 0;

    /**
     * @brief Insert a new product listing.
     */
    virtual Product CreateProduct(const Product& product) = 0;

    /**
     * @brief Update existing product listing.
     */
    virtual Product UpdateProduct(const Product& product) = 0;

    /**
     * @brief Delete a product listing.
     */
    virtual bool DeleteProduct(int64_t id) = 0;
};

} // namespace srikanth::srikanthmart
