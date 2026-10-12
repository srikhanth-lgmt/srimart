#pragma once

#include <memory>
#include <vector>
#include "repository/IProductRepository.h"
#include "dto/ProductDto.h"

namespace srikanth::srikanthmart {

/**
 * @brief Product Service handling catalog browsing, search, and seller product management.
 */
class ProductService {
public:
    /**
     * @brief Constructor using dependency inversion.
     * @param product_repo Shared pointer to abstract product repository.
     */
    explicit ProductService(std::shared_ptr<IProductRepository> product_repo)
        : product_repo_(std::move(product_repo)) {}

    /**
     * @brief Browse and filter products.
     */
    std::vector<ProductResponseDto> GetProducts(const std::string& query, const std::string& category);

    /**
     * @brief Get product details by ID.
     */
    ProductResponseDto GetProductById(int64_t id);

    /**
     * @brief Get products listed by a seller.
     */
    std::vector<ProductResponseDto> GetProductsBySeller(int64_t seller_id);

    /**
     * @brief Create a new product listing by seller.
     */
    ProductResponseDto CreateProduct(int64_t seller_id, const CreateProductRequestDto& req);

    /**
     * @brief Update an existing product listing.
     */
    ProductResponseDto UpdateProduct(int64_t seller_id, int64_t product_id, const UpdateProductRequestDto& req);

    /**
     * @brief Delete a product listing (Seller owner or Admin).
     */
    void DeleteProduct(int64_t user_id, bool is_admin, int64_t product_id);

private:
    std::shared_ptr<IProductRepository> product_repo_;
};

} // namespace srikanth::srikanthmart
