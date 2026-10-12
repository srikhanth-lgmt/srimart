#pragma once

#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "model/Product.h"

namespace srikanth::srikanthmart {

struct CreateProductRequestDto {
    std::string name;
    std::string description;
    int64_t price_cents{0};
    int32_t stock_qty{0};
    std::string category;
    std::string image_url;
};

struct UpdateProductRequestDto {
    std::string name;
    std::string description;
    int64_t price_cents{0};
    int32_t stock_qty{0};
    std::string category;
    std::string image_url;
};

class ProductResponseDto {
public:
    int64_t id{0};
    int64_t seller_id{0};
    std::string name;
    std::string description;
    int64_t price_cents{0};
    std::string formatted_price;
    int32_t stock_qty{0};
    std::string category;
    std::string image_url;
    std::string created_at;

    [[nodiscard]] nlohmann::json ToJson() const {
        return nlohmann::json{
            {"id", id},
            {"seller_id", seller_id},
            {"name", name},
            {"description", description},
            {"price_cents", price_cents},
            {"formatted_price", formatted_price},
            {"stock_qty", stock_qty},
            {"category", category},
            {"image_url", image_url},
            {"created_at", created_at}
        };
    }

    // Declared here, defined after the class is complete (see below) so that
    // Builder may hold ProductResponseDto by value.
    class Builder;

    static ProductResponseDto FromEntity(const Product& product);
};

class ProductResponseDto::Builder {
public:
    Builder& SetId(int64_t id) { dto_.id = id; return *this; }
    Builder& SetSellerId(int64_t seller_id) { dto_.seller_id = seller_id; return *this; }
    Builder& SetName(std::string name) { dto_.name = std::move(name); return *this; }
    Builder& SetDescription(std::string desc) { dto_.description = std::move(desc); return *this; }
    Builder& SetPriceCents(int64_t cents) {
        dto_.price_cents = cents;
        dto_.formatted_price = Money(cents).ToFormattedString();
        return *this;
    }
    Builder& SetStockQty(int32_t qty) { dto_.stock_qty = qty; return *this; }
    Builder& SetCategory(std::string category) { dto_.category = std::move(category); return *this; }
    Builder& SetImageUrl(std::string url) { dto_.image_url = std::move(url); return *this; }
    Builder& SetCreatedAt(std::string created_at) { dto_.created_at = std::move(created_at); return *this; }

    [[nodiscard]] ProductResponseDto Build() const {
        return dto_;
    }
private:
    ProductResponseDto dto_;
};

inline ProductResponseDto ProductResponseDto::FromEntity(const Product& product) {
    return Builder()
        .SetId(product.id)
        .SetSellerId(product.seller_id)
        .SetName(product.name)
        .SetDescription(product.description)
        .SetPriceCents(product.price.GetCents())
        .SetStockQty(product.stock_qty)
        .SetCategory(product.category)
        .SetImageUrl(product.image_url)
        .SetCreatedAt(product.created_at)
        .Build();
}

} // namespace srikanth::srikanthmart
