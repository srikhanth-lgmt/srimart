#include "controller/CartController.h"
#include "util/JsonUtil.h"

namespace sudharshan::sudharshanmart {

std::shared_ptr<CartService> CartController::cart_service_ = nullptr;

void CartController::GetCart(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        if (GetSessionUserRole(req) != "BUYER") throw ForbiddenException("Buyer account required");
        auto summary = cart_service_->GetCart(user_id_opt.value());
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(summary.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void CartController::AddItem(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        if (GetSessionUserRole(req) != "BUYER") throw ForbiddenException("Buyer account required");
        auto json = JsonUtil::ParseRequestBody(req);
        AddToCartRequestDto dto;
        dto.product_id = json.value("product_id", 0LL);
        dto.quantity = json.value("quantity", 1);

        auto summary = cart_service_->AddItem(user_id_opt.value(), dto);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(summary.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void CartController::UpdateQuantity(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                    int64_t product_id) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        if (GetSessionUserRole(req) != "BUYER") throw ForbiddenException("Buyer account required");
        auto json = JsonUtil::ParseRequestBody(req);
        UpdateCartRequestDto dto;
        dto.quantity = json.value("quantity", 1);

        auto summary = cart_service_->UpdateQuantity(user_id_opt.value(), product_id, dto);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(summary.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void CartController::RemoveItem(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                int64_t product_id) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        if (GetSessionUserRole(req) != "BUYER") throw ForbiddenException("Buyer account required");
        auto summary = cart_service_->RemoveItem(user_id_opt.value(), product_id);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(summary.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void CartController::ClearCart(const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        if (GetSessionUserRole(req) != "BUYER") throw ForbiddenException("Buyer account required");
        cart_service_->ClearCart(user_id_opt.value());
        nlohmann::json data;
        data["message"] = "Cart cleared";
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(data));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

} // namespace sudharshan::sudharshanmart
