#pragma once

#include <drogon/HttpController.h>
#include "controller/BaseController.h"
#include "service/ReviewService.h"

namespace srikanth::srikanthmart {

class ReviewController : public drogon::HttpController<ReviewController>, public BaseController {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ReviewController::GetReviews, "/api/v1/products/{1}/reviews", drogon::Get, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::CorsFilter");
    ADD_METHOD_TO(ReviewController::AddReview, "/api/v1/products/{1}/reviews", drogon::Post, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::AuthFilter", "srikanth::srikanthmart::CorsFilter");
    METHOD_LIST_END

    void GetReviews(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    int64_t product_id);

    void AddReview(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                   int64_t product_id);

    static void SetReviewService(std::shared_ptr<ReviewService> service) {
        review_service_ = std::move(service);
    }

private:
    static std::shared_ptr<ReviewService> review_service_;
};

} // namespace srikanth::srikanthmart
