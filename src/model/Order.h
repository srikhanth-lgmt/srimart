#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "model/Money.h"

namespace srikanth::srikanthmart {

enum class OrderStatus {
    kPending,
    kConfirmed,
    kShipped,
    kDelivered,
    kCancelled
};

inline std::string OrderStatusToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::kPending: return "PENDING";
        case OrderStatus::kConfirmed: return "CONFIRMED";
        case OrderStatus::kShipped: return "SHIPPED";
        case OrderStatus::kDelivered: return "DELIVERED";
        case OrderStatus::kCancelled: return "CANCELLED";
    }
    return "PENDING";
}

inline OrderStatus StringToOrderStatus(const std::string& str) {
    if (str == "CONFIRMED") return OrderStatus::kConfirmed;
    if (str == "SHIPPED") return OrderStatus::kShipped;
    if (str == "DELIVERED") return OrderStatus::kDelivered;
    if (str == "CANCELLED") return OrderStatus::kCancelled;
    return OrderStatus::kPending;
}

/**
 * @brief Order line item entity.
 */
struct OrderItem {
    int64_t id{0};
    int64_t order_id{0};
    int64_t product_id{0};
    int32_t quantity{1};
    Money unit_price{0};
    std::string created_at;

    // Joined Product metadata
    std::string product_name;
    int64_t seller_id{0};
};

/**
 * @brief Order entity.
 */
struct Order {
    int64_t id{0};
    int64_t buyer_id{0};
    OrderStatus status{OrderStatus::kPending};
    Money total_amount{0};
    std::string created_at;
    std::vector<OrderItem> items;
};

} // namespace srikanth::srikanthmart
