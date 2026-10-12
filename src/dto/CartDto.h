#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "model/CartItem.h"

namespace srikanth::srikanthmart {

struct AddToCartRequestDto {
    int64_t product_id{0};
    int32_t quantity{1};
};

struct UpdateCartRequestDto {
    int32_t quantity{1};
};

struct CartItemResponseDto {
    int64_t id{0};
    int64_t product_id{0};
    std::string product_name;
    int32_t quantity{1};
    int64_t unit_price_cents{0};
    int64_t line_total_cents{0};
    std::string formatted_unit_price;
    std::string formatted_line_total;
    std::string product_image_url;

    [[nodiscard]] nlohmann::json ToJson() const {
        return nlohmann::json{
            {"id", id},
            {"product_id", product_id},
            {"product_name", product_name},
            {"quantity", quantity},
            {"unit_price_cents", unit_price_cents},
            {"line_total_cents", line_total_cents},
            {"formatted_unit_price", formatted_unit_price},
            {"formatted_line_total", formatted_line_total},
            {"product_image_url", product_image_url}
        };
    }
};

class CartSummaryResponseDto {
public:
    std::vector<CartItemResponseDto> items;
    int32_t total_items_count{0};
    int64_t total_amount_cents{0};
    std::string formatted_total;

    [[nodiscard]] nlohmann::json ToJson() const {
        nlohmann::json items_arr = nlohmann::json::array();
        for (const auto& item : items) {
            items_arr.push_back(item.ToJson());
        }
        return nlohmann::json{
            {"items", items_arr},
            {"total_items_count", total_items_count},
            {"total_amount_cents", total_amount_cents},
            {"formatted_total", formatted_total}
        };
    }

    class Builder {
    public:
        Builder& AddItem(const CartItem& item) {
            CartItemResponseDto d;
            d.id = item.id;
            d.product_id = item.product_id;
            d.product_name = item.product_name;
            d.quantity = item.quantity;
            d.unit_price_cents = item.unit_price.GetCents();
            int64_t line_total = item.unit_price.GetCents() * item.quantity;
            d.line_total_cents = line_total;
            d.formatted_unit_price = item.unit_price.ToFormattedString();
            d.formatted_line_total = Money(line_total).ToFormattedString();
            d.product_image_url = item.product_image_url;

            total_cents_ += line_total;
            count_ += item.quantity;
            items_.push_back(d);
            return *this;
        }

        [[nodiscard]] CartSummaryResponseDto Build() const {
            CartSummaryResponseDto summary;
            summary.items = items_;
            summary.total_items_count = count_;
            summary.total_amount_cents = total_cents_;
            summary.formatted_total = Money(total_cents_).ToFormattedString();
            return summary;
        }

    private:
        std::vector<CartItemResponseDto> items_;
        int32_t count_{0};
        int64_t total_cents_{0};
    };
};

} // namespace srikanth::srikanthmart
