#include "controller/ReviewController.h"
#include "util/JsonUtil.h"

namespace srikanth::srikanthmart {

std::shared_ptr<ReviewService> ReviewController::review_service_ = nullptr;

void ReviewController::GetReviews(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                  int64_t product_id) {
    std::string request_id = GetRequestId(req);
    try {
        auto reviews = review_service_->GetProductReviews(product_id);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& r : reviews) {
            arr.push_back(r.ToJson());
        }
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(arr));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void ReviewController::AddReview(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                 int64_t product_id) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required to submit a review");
        }
        if (GetSessionUserRole(req) != "BUYER") throw ForbiddenException("Buyer account required");
        auto json = JsonUtil::ParseRequestBody(req);
        CreateReviewRequestDto dto;
        dto.rating = json.value("rating", 5);
        dto.comment = json.value("comment", "");

        auto created = review_service_->AddReview(user_id_opt.value(), product_id, dto);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(created.ToJson()), drogon::k201Created);
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

} // namespace srikanth::srikanthmart
