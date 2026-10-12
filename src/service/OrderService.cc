#include "service/OrderService.h"
#include "util/ValidationUtil.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace srikanth::srikanthmart {

OrderResponseDto OrderService::Checkout(int64_t buyer_id, const CheckoutRequestDto& req) {
    // 1. Validation
    ValidationUtil::ValidateNonEmpty(req.shipping_address, "shipping_address", "Shipping address is required");

    if (!req.mock_payment_confirmed) {
        throw ValidationException("Payment confirmation was not approved", "mock_payment_confirmed");
    }

    // 2. Fetch cart
    auto cart_items = cart_repo_->GetCart(buyer_id);
    if (cart_items.empty()) {
        throw ValidationException("Cannot checkout with an empty cart");
    }

    // 3. Compute total amount
    int64_t total_cents = 0;
    for (const auto& item : cart_items) {
        total_cents += item.unit_price.GetCents() * item.quantity;
    }

    // 4. Create Order atomically
    Order order = order_repo_->CreateOrder(buyer_id, cart_items, total_cents);

    // 5. Clear cart
    cart_repo_->ClearCart(buyer_id);

    spdlog::info("Order #{} placed successfully by buyer {} for total {} cents", order.id, buyer_id, total_cents);
    return OrderResponseDto::FromEntity(order);
}

OrderResponseDto OrderService::GetOrderById(int64_t order_id, int64_t requester_id, bool is_admin) {
    auto order_opt = order_repo_->FindById(order_id);
    if (!order_opt.has_value()) {
        throw NotFoundException("Order not found");
    }

    const Order& order = order_opt.value();
    if (!is_admin && order.buyer_id != requester_id) {
        // Check if requester is seller of one of the items
        bool is_seller = false;
        for (const auto& item : order.items) {
            if (item.seller_id == requester_id) {
                is_seller = true;
                break;
            }
        }
        if (!is_seller) {
            throw ForbiddenException("You are not authorized to view this order");
        }
    }

    return OrderResponseDto::FromEntity(order);
}

std::vector<OrderResponseDto> OrderService::GetBuyerOrders(int64_t buyer_id) {
    auto orders = order_repo_->FindByBuyerId(buyer_id);
    std::vector<OrderResponseDto> dtos;
    dtos.reserve(orders.size());
    for (const auto& o : orders) {
        dtos.push_back(OrderResponseDto::FromEntity(o));
    }
    return dtos;
}

std::vector<OrderResponseDto> OrderService::GetSellerIncomingOrders(int64_t seller_id) {
    auto orders = order_repo_->FindIncomingForSeller(seller_id);
    std::vector<OrderResponseDto> dtos;
    dtos.reserve(orders.size());
    for (const auto& o : orders) {
        dtos.push_back(OrderResponseDto::FromEntity(o));
    }
    return dtos;
}

std::vector<OrderResponseDto> OrderService::GetAllOrders() {
    auto orders = order_repo_->FindAll();
    std::vector<OrderResponseDto> dtos;
    dtos.reserve(orders.size());
    for (const auto& o : orders) {
        dtos.push_back(OrderResponseDto::FromEntity(o));
    }
    return dtos;
}

OrderResponseDto OrderService::UpdateOrderStatus(int64_t order_id, int64_t requester_id, bool is_admin, const UpdateOrderStatusRequestDto& req) {
    ValidationUtil::ValidateNonEmpty(req.status, "status", "Status cannot be empty");

    auto order_opt = order_repo_->FindById(order_id);
    if (!order_opt.has_value()) {
        throw NotFoundException("Order not found");
    }
    const auto& order = order_opt.value();
    const bool owns_item = std::any_of(order.items.begin(), order.items.end(), [requester_id](const auto& item) { return item.seller_id == requester_id; });
    if (!is_admin && !owns_item) throw ForbiddenException("You are not authorized to update this order");

    OrderStatus new_status = StringToOrderStatus(req.status);
    order_repo_->UpdateOrderStatus(order_id, new_status);

    auto updated_opt = order_repo_->FindById(order_id);
    spdlog::info("Order #{} status updated to {}", order_id, req.status);
    return OrderResponseDto::FromEntity(updated_opt.value());
}

SellerDashboardDto OrderService::GetSellerDashboard(int64_t seller_id) {
    int64_t count = order_repo_->GetSellerOrderCount(seller_id);
    int64_t revenue = order_repo_->GetSellerRevenue(seller_id);

    SellerDashboardDto dto;
    dto.total_orders_count = count;
    dto.total_revenue_cents = revenue;
    dto.formatted_revenue = Money(revenue).ToFormattedString();
    return dto;
}

} // namespace srikanth::srikanthmart
