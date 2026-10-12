#pragma once

#include <vector>
#include <memory>
#include "model/Review.h"

namespace srikanth::srikanthmart {

class IReviewRepository {
public:
    virtual ~IReviewRepository() = default;

    /**
     * @brief Persist a review for a product.
     */
    virtual Review CreateReview(const Review& review) = 0;

    /**
     * @brief Retrieve all reviews for a product.
     */
    virtual std::vector<Review> FindByProductId(int64_t product_id) = 0;

    /**
     * @brief Check whether buyer has a completed/delivered order for the product.
     */
    virtual bool HasUserPurchasedProduct(int64_t user_id, int64_t product_id) = 0;
};

class IWishlistRepository {
public:
    virtual ~IWishlistRepository() = default;

    virtual std::vector<WishlistItem> GetWishlist(int64_t user_id) = 0;
    virtual void AddToWishlist(int64_t user_id, int64_t product_id) = 0;
    virtual void RemoveFromWishlist(int64_t user_id, int64_t product_id) = 0;
};

} // namespace srikanth::srikanthmart
