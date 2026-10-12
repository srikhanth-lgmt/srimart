#include "service/CartService.h"
#include "util/ValidationUtil.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

CartSummaryResponseDto CartService::GetCart(int64_t user_id) {
    auto items = cart_repo_->GetCart(user_id);
    CartSummaryResponseDto::Builder builder;
    for (const auto& item : items) {
        builder.AddItem(item);
    }
    return builder.Build();
}

CartSummaryResponseDto CartService::AddItem(int64_t user_id, const AddToCartRequestDto& req) {
    if (req.product_id <= 0) {
        throw ValidationException("Invalid product ID", "product_id");
    }
    if (req.quantity <= 0) {
        throw ValidationException("Quantity must be greater than zero", "quantity");
    }

    auto product_opt = product_repo_->FindById(req.product_id);
    if (!product_opt.has_value()) {
        throw NotFoundException("Product not found");
    }

    if (product_opt.value().stock_qty < req.quantity) {
        throw ConflictException("Requested quantity exceeds available stock (" +
                                std::to_string(product_opt.value().stock_qty) + " available)");
    }

    cart_repo_->AddOrUpdateItem(user_id, req.product_id, req.quantity);
    spdlog::info("User {} added product {} (qty: {}) to cart", user_id, req.product_id, req.quantity);
    return GetCart(user_id);
}

CartSummaryResponseDto CartService::UpdateQuantity(int64_t user_id, int64_t product_id, const UpdateCartRequestDto& req) {
    if (product_id <= 0) {
        throw ValidationException("Invalid product ID", "product_id");
    }
    if (req.quantity <= 0) {
        // If 0, treat as remove
        cart_repo_->RemoveItem(user_id, product_id);
    } else {
        auto product_opt = product_repo_->FindById(product_id);
        if (!product_opt.has_value()) {
            throw NotFoundException("Product not found");
        }
        if (product_opt.value().stock_qty < req.quantity) {
            throw ConflictException("Requested quantity exceeds available stock");
        }
        cart_repo_->RemoveItem(user_id, product_id);
        cart_repo_->AddOrUpdateItem(user_id, product_id, req.quantity);
    }
    return GetCart(user_id);
}

CartSummaryResponseDto CartService::RemoveItem(int64_t user_id, int64_t product_id) {
    if (product_id <= 0) {
        throw ValidationException("Invalid product ID", "product_id");
    }
    cart_repo_->RemoveItem(user_id, product_id);
    return GetCart(user_id);
}

void CartService::ClearCart(int64_t user_id) {
    cart_repo_->ClearCart(user_id);
}

} // namespace srikanth::srikanthmart
