#include "service/ProductService.h"
#include "util/ValidationUtil.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

std::vector<ProductResponseDto> ProductService::GetProducts(const std::string& query, const std::string& category) {
    auto products = product_repo_->FindAll(query, category);
    std::vector<ProductResponseDto> dtos;
    dtos.reserve(products.size());
    for (const auto& p : products) {
        dtos.push_back(ProductResponseDto::FromEntity(p));
    }
    return dtos;
}

ProductResponseDto ProductService::GetProductById(int64_t id) {
    if (id <= 0) {
        throw ValidationException("Invalid product ID", "id");
    }
    auto p_opt = product_repo_->FindById(id);
    if (!p_opt.has_value()) {
        throw NotFoundException("Product not found");
    }
    return ProductResponseDto::FromEntity(p_opt.value());
}

std::vector<ProductResponseDto> ProductService::GetProductsBySeller(int64_t seller_id) {
    auto products = product_repo_->FindBySellerId(seller_id);
    std::vector<ProductResponseDto> dtos;
    dtos.reserve(products.size());
    for (const auto& p : products) {
        dtos.push_back(ProductResponseDto::FromEntity(p));
    }
    return dtos;
}

ProductResponseDto ProductService::CreateProduct(int64_t seller_id, const CreateProductRequestDto& req) {
    // 1. Validations at top of method
    ValidationUtil::ValidateNonEmpty(req.name, "name", "Product name cannot be empty");
    ValidationUtil::ValidateNonEmpty(req.description, "description", "Product description cannot be empty");
    ValidationUtil::ValidatePositiveAmount(req.price_cents, "price_cents");
    ValidationUtil::ValidateNonNegativeQuantity(req.stock_qty, "stock_qty");
    ValidationUtil::ValidateNonEmpty(req.category, "category", "Category cannot be empty");

    // 2. Build product entity
    Product p;
    p.seller_id = seller_id;
    p.name = ValidationUtil::SanitizeHtml(req.name);
    p.description = ValidationUtil::SanitizeHtml(req.description);
    p.price = Money(req.price_cents);
    p.stock_qty = req.stock_qty;
    p.category = ValidationUtil::SanitizeHtml(req.category);
    p.image_url = req.image_url.empty()
        ? "https://images.unsplash.com/photo-1526170375885-4d8ecf77b99f?w=600&auto=format&fit=crop&q=80"
        : req.image_url;

    // 3. Persist
    Product created = product_repo_->CreateProduct(p);
    spdlog::info("Seller {} created product '{}' with price {} cents", seller_id, created.name, created.price.GetCents());
    return ProductResponseDto::FromEntity(created);
}

ProductResponseDto ProductService::UpdateProduct(int64_t seller_id, int64_t product_id, const UpdateProductRequestDto& req) {
    if (product_id <= 0) {
        throw ValidationException("Invalid product ID", "id");
    }
    ValidationUtil::ValidateNonEmpty(req.name, "name", "Product name cannot be empty");
    ValidationUtil::ValidatePositiveAmount(req.price_cents, "price_cents");
    ValidationUtil::ValidateNonNegativeQuantity(req.stock_qty, "stock_qty");

    auto existing_opt = product_repo_->FindById(product_id);
    if (!existing_opt.has_value()) {
        throw NotFoundException("Product not found");
    }

    if (existing_opt.value().seller_id != seller_id) {
        throw ForbiddenException("You do not have permission to modify this product listing");
    }

    Product p = existing_opt.value();
    p.name = ValidationUtil::SanitizeHtml(req.name);
    p.description = ValidationUtil::SanitizeHtml(req.description);
    p.price = Money(req.price_cents);
    p.stock_qty = req.stock_qty;
    p.category = req.category;
    if (!req.image_url.empty()) {
        p.image_url = req.image_url;
    }

    Product updated = product_repo_->UpdateProduct(p);
    spdlog::info("Product {} updated by seller {}", product_id, seller_id);
    return ProductResponseDto::FromEntity(updated);
}

void ProductService::DeleteProduct(int64_t user_id, bool is_admin, int64_t product_id) {
    auto existing_opt = product_repo_->FindById(product_id);
    if (!existing_opt.has_value()) {
        throw NotFoundException("Product not found");
    }

    if (!is_admin && existing_opt.value().seller_id != user_id) {
        throw ForbiddenException("You do not have permission to delete this product listing");
    }

    if (!product_repo_->DeleteProduct(product_id)) {
        throw InternalServerException("Failed to delete product");
    }
    spdlog::info("Product {} removed by user {} (isAdmin: {})", product_id, user_id, is_admin);
}

} // namespace srikanth::srikanthmart
