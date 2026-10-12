#pragma once

#include <memory>
#include <vector>
#include "repository/IReviewRepository.h"
#include "repository/IProductRepository.h"
#include "dto/ReviewDto.h"

namespace srikanth::srikanthmart {

/**
 * @brief Review Service managing star ratings and customer feedback.
 */
class ReviewService {
public:
    explicit ReviewService(std::shared_ptr<IReviewRepository> review_repo,
                           std::shared_ptr<IProductRepository> product_repo)
        : review_repo_(std::move(review_repo)),
          product_repo_(std::move(product_repo)) {}

    /**
     * @brief Post a product review after purchase verification.
     */
    ReviewResponseDto AddReview(int64_t user_id, int64_t product_id, const CreateReviewRequestDto& req);

    /**
     * @brief Retrieve reviews for a product.
     */
    std::vector<ReviewResponseDto> GetProductReviews(int64_t product_id);

private:
    std::shared_ptr<IReviewRepository> review_repo_;
    std::shared_ptr<IProductRepository> product_repo_;
};

} // namespace srikanth::srikanthmart
