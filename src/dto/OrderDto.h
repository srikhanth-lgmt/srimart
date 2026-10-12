#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "model/Order.h"

namespace srikanth::srikanthmart {

struct CheckoutRequestDto {
    std::string shipping_address;
    std::string mock_payment_method; // e.g., "MOCK_CARD", "MOCK_UPI", "MOCK_NETBANKING"
    bool mock_payment_confirmed{true};
};

struct UpdateOrderStatusRequestDto {
    std::string status; // PENDING, CONFIRMED, SHIPPED, DELIVERED, CANCELLED
};

struct OrderItemResponseDto {
    int64_t id{0};
    int64_t product_id{0};
    std::string product_name;
    int32_t quantity{1};
    int64_t unit_price_cents{0};
    std::string formatted_unit_price;

    [[nodiscard]] nlohmann::json ToJson() const {
        return nlohmann::json{
            {"id", id},
            {"product_id", product_id},
            {"product_name", product_name},
            {"quantity", quantity},
            {"unit_price_cents", unit_price_cents},
            {"formatted_unit_price", formatted_unit_price}
        };
    }
};

class OrderResponseDto {
public:
    int64_t id{0};
    int64_t buyer_id{0};
    std::string status;
    int64_t total_amount_cents{0};
    std::string formatted_total;
    std::string created_at;
    std::vector<OrderItemResponseDto> items;

    [[nodiscard]] nlohmann::json ToJson() const {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& it : items) {
            arr.push_back(it.ToJson());
        }
        return nlohmann::json{
            {"id", id},
            {"buyer_id", buyer_id},
            {"status", status},
            {"total_amount_cents", total_amount_cents},
            {"formatted_total", formatted_total},
            {"created_at", created_at},
            {"items", arr}
        };
    }

    // Declared here, defined after the class is complete (see below) so that
    // Builder may hold OrderResponseDto by value.
    class Builder;

    static OrderResponseDto FromEntity(const Order& order);
};

class OrderResponseDto::Builder {
public:
    Builder& SetId(int64_t id) { dto_.id = id; return *this; }
    Builder& SetBuyerId(int64_t buyer_id) { dto_.buyer_id = buyer_id; return *this; }
    Builder& SetStatus(std::string status) { dto_.status = std::move(status); return *this; }
    Builder& SetTotalCents(int64_t total_cents) {
        dto_.total_amount_cents = total_cents;
        dto_.formatted_total = Money(total_cents).ToFormattedString();
        return *this;
    }
    Builder& SetCreatedAt(std::string created_at) { dto_.created_at = std::move(created_at); return *this; }
    Builder& AddItem(const OrderItem& item) {
        OrderItemResponseDto item_dto;
        item_dto.id = item.id;
        item_dto.product_id = item.product_id;
        item_dto.product_name = item.product_name;
        item_dto.quantity = item.quantity;
        item_dto.unit_price_cents = item.unit_price.GetCents();
        item_dto.formatted_unit_price = item.unit_price.ToFormattedString();
        dto_.items.push_back(item_dto);
        return *this;
    }

    [[nodiscard]] OrderResponseDto Build() const {
        return dto_;
    }
private:
    OrderResponseDto dto_;
};

inline OrderResponseDto OrderResponseDto::FromEntity(const Order& order) {
    Builder b;
    b.SetId(order.id)
     .SetBuyerId(order.buyer_id)
     .SetStatus(OrderStatusToString(order.status))
     .SetTotalCents(order.total_amount.GetCents())
     .SetCreatedAt(order.created_at);
    for (const auto& item : order.items) {
        b.AddItem(item);
    }
    return b.Build();
}

/**
 * @brief Seller Sales Dashboard Statistics DTO.
 */
struct SellerDashboardDto {
    int64_t total_orders_count{0};
    int64_t total_revenue_cents{0};
    std::string formatted_revenue;
    int64_t total_products_listed{0};

    [[nodiscard]] nlohmann::json ToJson() const {
        return nlohmann::json{
            {"total_orders_count", total_orders_count},
            {"total_revenue_cents", total_revenue_cents},
            {"formatted_revenue", formatted_revenue},
            {"total_products_listed", total_products_listed}
        };
    }
};

} // namespace srikanth::srikanthmart
