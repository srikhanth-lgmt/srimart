#pragma once

#include <cstdint>
#include <string>
#include "model/Money.h"

namespace srikanth::srikanthmart {

/**
 * @brief Cart item entity with associated product snapshot.
 */
struct CartItem {
    int64_t id{0};
    int64_t user_id{0};
    int64_t product_id{0};
    int32_t quantity{1};
    std::string created_at;

    // Joined Product Details for convenience
    std::string product_name;
    Money unit_price{0};
    std::string product_image_url;
    std::string product_category;
};

} // namespace srikanth::srikanthmart
