#pragma once

#include <memory>
#include "repository/ICartRepository.h"
#include "repository/IProductRepository.h"
#include "dto/CartDto.h"

namespace srikanth::srikanthmart {

/**
 * @brief Cart Service managing shopping cart state and running totals.
 */
class CartService {
public:
    explicit CartService(std::shared_ptr<ICartRepository> cart_repo,
                         std::shared_ptr<IProductRepository> product_repo)
        : cart_repo_(std::move(cart_repo)),
          product_repo_(std::move(product_repo)) {}

    /**
     * @brief Retrieve user's current shopping cart with calculated totals.
     */
    CartSummaryResponseDto GetCart(int64_t user_id);

    /**
     * @brief Add item or increment quantity in cart.
     */
    CartSummaryResponseDto AddItem(int64_t user_id, const AddToCartRequestDto& req);

    /**
     * @brief Update item quantity in cart.
     */
    CartSummaryResponseDto UpdateQuantity(int64_t user_id, int64_t product_id, const UpdateCartRequestDto& req);

    /**
     * @brief Remove item from cart.
     */
    CartSummaryResponseDto RemoveItem(int64_t user_id, int64_t product_id);

    /**
     * @brief Clear all cart items.
     */
    void ClearCart(int64_t user_id);

private:
    std::shared_ptr<ICartRepository> cart_repo_;
    std::shared_ptr<IProductRepository> product_repo_;
};

} // namespace srikanth::srikanthmart
