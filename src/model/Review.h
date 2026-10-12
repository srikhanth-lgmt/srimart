#pragma once

#include <cstdint>
#include <string>

namespace srikanth::srikanthmart {

/**
 * @brief Review entity with rating and comment.
 */
struct Review {
    int64_t id{0};
    int64_t product_id{0};
    int64_t user_id{0};
    int32_t rating{5};
    std::string comment;
    std::string created_at;

    // Joined User Details
    std::string reviewer_name;
};

/**
 * @brief Wishlist item entity.
 */
struct WishlistItem {
    int64_t id{0};
    int64_t user_id{0};
    int64_t product_id{0};
    std::string created_at;

    // Joined Product
    std::string product_name;
    int64_t price_cents{0};
    std::string product_image_url;
};

} // namespace srikanth::srikanthmart
