#include "controller/OrderController.h"
#include "util/JsonUtil.h"

namespace sudharshan::sudharshanmart {

std::shared_ptr<OrderService> OrderController::order_service_ = nullptr;

void OrderController::Checkout(const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        if (GetSessionUserRole(req) != "BUYER") throw ForbiddenException("Buyer account required");
        auto json = JsonUtil::ParseRequestBody(req);
        CheckoutRequestDto dto;
        dto.shipping_address = json.value("shipping_address", "");
        dto.mock_payment_method = json.value("mock_payment_method", "MOCK_UPI");
        dto.mock_payment_confirmed = json.value("mock_payment_confirmed", true);

        auto order = order_service_->Checkout(user_id_opt.value(), dto);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(order.ToJson()), drogon::k201Created);
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void OrderController::GetBuyerOrders(const drogon::HttpRequestPtr& req,
                                     std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        if (GetSessionUserRole(req) != "BUYER") throw ForbiddenException("Buyer account required");
        auto orders = order_service_->GetBuyerOrders(user_id_opt.value());
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& o : orders) {
            arr.push_back(o.ToJson());
        }
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(arr));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void OrderController::GetSellerOrders(const drogon::HttpRequestPtr& req,
                                      std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        if (GetSessionUserRole(req) != "SELLER") throw ForbiddenException("Seller account required");
        auto orders = order_service_->GetSellerIncomingOrders(user_id_opt.value());
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& o : orders) {
            arr.push_back(o.ToJson());
        }
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(arr));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void OrderController::GetSellerDashboard(const drogon::HttpRequestPtr& req,
                                         std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        if (GetSessionUserRole(req) != "SELLER") throw ForbiddenException("Seller account required");
        auto dashboard = order_service_->GetSellerDashboard(user_id_opt.value());
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(dashboard.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void OrderController::GetOrder(const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                               int64_t order_id) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        std::string role = GetSessionUserRole(req);
        bool is_admin = (role == "ADMIN");

        auto order = order_service_->GetOrderById(order_id, user_id_opt.value(), is_admin);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(order.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void OrderController::UpdateOrderStatus(const drogon::HttpRequestPtr& req,
                                        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                        int64_t order_id) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        auto json = JsonUtil::ParseRequestBody(req);
        UpdateOrderStatusRequestDto dto;
        dto.status = json.value("status", "CONFIRMED");

        const auto role = GetSessionUserRole(req);
        if (role != "SELLER" && role != "ADMIN") throw ForbiddenException("Seller account required");
        auto updated = order_service_->UpdateOrderStatus(order_id, user_id_opt.value(), role == "ADMIN", dto);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(updated.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

} // namespace sudharshan::sudharshanmart
