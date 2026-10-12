#pragma once

#include <optional>
#include <vector>
#include <memory>
#include "model/Order.h"
#include "model/CartItem.h"

namespace srikanth::srikanthmart {

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;

    /**
     * @brief Creates an order atomically inside a transaction with line items.
     */
    virtual Order CreateOrder(int64_t buyer_id, const std::vector<CartItem>& items, int64_t total_amount_cents) = 0;

    /**
     * @brief Find order by ID.
     */
    virtual std::optional<Order> FindById(int64_t order_id) = 0;

    /**
     * @brief Find past orders placed by a specific buyer.
     */
    virtual std::vector<Order> FindByBuyerId(int64_t buyer_id) = 0;

    /**
     * @brief Find incoming orders containing products owned by a seller.
     */
    virtual std::vector<Order> FindIncomingForSeller(int64_t seller_id) = 0;

    /**
     * @brief Find all orders in system (Admin view).
     */
    virtual std::vector<Order> FindAll() = 0;

    /**
     * @brief Update status in workflow (PENDING -> CONFIRMED -> SHIPPED -> DELIVERED).
     */
    virtual bool UpdateOrderStatus(int64_t order_id, OrderStatus status) = 0;

    /**
     * @brief Calculate seller total revenue in cents.
     */
    virtual int64_t GetSellerRevenue(int64_t seller_id) = 0;

    /**
     * @brief Count seller orders.
     */
    virtual int64_t GetSellerOrderCount(int64_t seller_id) = 0;
};

} // namespace srikanth::srikanthmart
