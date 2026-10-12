#include "service/ReviewService.h"
#include "util/ValidationUtil.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

ReviewResponseDto ReviewService::AddReview(int64_t user_id, int64_t product_id, const CreateReviewRequestDto& req) {
    if (product_id <= 0) {
        throw ValidationException("Invalid product ID", "product_id");
    }
    ValidationUtil::ValidateRating(req.rating);
    ValidationUtil::ValidateNonEmpty(req.comment, "comment", "Review comment cannot be empty");

    auto product_opt = product_repo_->FindById(product_id);
    if (!product_opt.has_value()) {
        throw NotFoundException("Product not found");
    }

    // F8: Product reviews and star ratings on completed orders
    if (!review_repo_->HasUserPurchasedProduct(user_id, product_id)) {
        throw ForbiddenException("Reviews can only be submitted for completed orders you have purchased");
    }

    Review review;
    review.product_id = product_id;
    review.user_id = user_id;
    review.rating = req.rating;
    review.comment = ValidationUtil::SanitizeHtml(req.comment);

    Review created = review_repo_->CreateReview(review);
    spdlog::info("Review posted for product {} by user {}", product_id, user_id);
    return ReviewResponseDto::FromEntity(created);
}

std::vector<ReviewResponseDto> ReviewService::GetProductReviews(int64_t product_id) {
    if (product_id <= 0) {
        throw ValidationException("Invalid product ID", "product_id");
    }
    auto reviews = review_repo_->FindByProductId(product_id);
    std::vector<ReviewResponseDto> dtos;
    dtos.reserve(reviews.size());
    for (const auto& r : reviews) {
        dtos.push_back(ReviewResponseDto::FromEntity(r));
    }
    return dtos;
}

} // namespace srikanth::srikanthmart
