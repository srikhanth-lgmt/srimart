#include "controller/AdminController.h"
#include "dto/UserDto.h"

namespace srikanth::srikanthmart {

std::shared_ptr<IUserRepository> AdminController::user_repo_ = nullptr;
std::shared_ptr<OrderService> AdminController::order_service_ = nullptr;
std::shared_ptr<ProductService> AdminController::product_service_ = nullptr;

void AdminController::EnforceAdminRole(const drogon::HttpRequestPtr& req) {
    std::string role = GetSessionUserRole(req);
    if (role != "ADMIN") {
        throw ForbiddenException("Administrative privileges required");
    }
}

void AdminController::GetAllUsers(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        EnforceAdminRole(req);
        auto users = user_repo_->FindAll();
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& u : users) {
            arr.push_back(UserResponseDto::FromEntity(u).ToJson());
        }
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(arr));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void AdminController::GetAllOrders(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        EnforceAdminRole(req);
        auto orders = order_service_->GetAllOrders();
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

void AdminController::ModerateProduct(const drogon::HttpRequestPtr& req,
                                      std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                      int64_t product_id) {
    std::string request_id = GetRequestId(req);
    try {
        EnforceAdminRole(req);
        product_service_->DeleteProduct(0, true, product_id);
        nlohmann::json data;
        data["message"] = "Product moderated and removed successfully";
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(data));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

} // namespace srikanth::srikanthmart
