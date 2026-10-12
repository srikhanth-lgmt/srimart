#pragma once

#include <vector>
#include <memory>
#include "model/CartItem.h"

namespace srikanth::srikanthmart {

class ICartRepository {
public:
    virtual ~ICartRepository() = default;

    /**
     * @brief Retrieve all active cart items for a given user.
     */
    virtual std::vector<CartItem> GetCart(int64_t user_id) = 0;

    /**
     * @brief Add or update item quantity in cart.
     */
    virtual CartItem AddOrUpdateItem(int64_t user_id, int64_t product_id, int32_t quantity) = 0;

    /**
     * @brief Remove a specific product from user's cart.
     */
    virtual bool RemoveItem(int64_t user_id, int64_t product_id) = 0;

    /**
     * @brief Clear all cart items for a user upon checkout.
     */
    virtual void ClearCart(int64_t user_id) = 0;
};

} // namespace srikanth::srikanthmart
