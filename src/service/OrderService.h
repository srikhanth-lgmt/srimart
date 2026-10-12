#pragma once

#include <memory>
#include <vector>
#include "repository/IOrderRepository.h"
#include "repository/ICartRepository.h"
#include "dto/OrderDto.h"

namespace sudharshan::sudharshanmart {

/**
 * @brief Order Service managing checkout flow, mock payment validation,
 * order status state transitions, and seller revenue analytics.
 */
class OrderService {
public:
    explicit OrderService(std::shared_ptr<IOrderRepository> order_repo,
                          std::shared_ptr<ICartRepository> cart_repo)
        : order_repo_(std::move(order_repo)),
          cart_repo_(std::move(cart_repo)) {}

    /**
     * @brief Process checkout from cart with mock payment confirmation.
     */
    OrderResponseDto Checkout(int64_t buyer_id, const CheckoutRequestDto& req);

    /**
     * @brief Fetch order by ID.
     */
    OrderResponseDto GetOrderById(int64_t order_id, int64_t requester_id, bool is_admin);

    /**
     * @brief Get past orders for a buyer.
     */
    std::vector<OrderResponseDto> GetBuyerOrders(int64_t buyer_id);

    /**
     * @brief Get incoming orders for a seller's products.
     */
    std::vector<OrderResponseDto> GetSellerIncomingOrders(int64_t seller_id);

    /**
     * @brief Get all orders in system (Admin).
     */
    std::vector<OrderResponseDto> GetAllOrders();

    /**
     * @brief Advance order status workflow (PENDING -> CONFIRMED -> SHIPPED -> DELIVERED).
     */
    OrderResponseDto UpdateOrderStatus(int64_t order_id, int64_t requester_id, bool is_admin, const UpdateOrderStatusRequestDto& req);

    /**
     * @brief Compute seller sales metrics (total revenue and count).
     */
    SellerDashboardDto GetSellerDashboard(int64_t seller_id);

private:
    std::shared_ptr<IOrderRepository> order_repo_;
    std::shared_ptr<ICartRepository> cart_repo_;
};

} // namespace sudharshan::sudharshanmart
