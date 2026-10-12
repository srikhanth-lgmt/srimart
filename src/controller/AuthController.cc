#include "controller/AuthController.h"
#include "util/JsonUtil.h"

namespace srikanth::srikanthmart {

std::shared_ptr<AuthService> AuthController::auth_service_ = nullptr;

void AuthController::Register(const drogon::HttpRequestPtr& req,
                              std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto json = JsonUtil::ParseRequestBody(req);
        RegisterUserRequestDto dto;
        dto.name = json.value("name", "");
        dto.email = json.value("email", "");
        dto.password = json.value("password", "");
        dto.role = json.value("role", "BUYER");
        dto.accepted_terms = json.value("accepted_terms", false);

        UserResponseDto created = auth_service_->Register(dto);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(created.ToJson()), drogon::k201Created);
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void AuthController::Login(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto json = JsonUtil::ParseRequestBody(req);
        LoginRequestDto dto;
        dto.email = json.value("email", "");
        dto.password = json.value("password", "");

        UserResponseDto user = auth_service_->Login(dto);

        // Discard pre-auth session and issue fresh one (Spec rule 3)
        auto session = req->session();
        if (session) {
            session->clear();
            session->changeSessionIdToClient();
            session->insert("user_id", user.id);
            session->insert("user_email", user.email);
            session->insert("user_role", user.role);
        }

        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(user.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void AuthController::Logout(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto session = req->session();
    if (session) {
        session->clear();
        session->changeSessionIdToClient();
    }
    nlohmann::json data;
    data["message"] = "Logged out successfully";
    auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(data));
    callback(resp);
}

void AuthController::GetCurrentUser(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("No active session");
        }
        UserResponseDto user = auth_service_->GetUserById(user_id_opt.value());
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(user.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

} // namespace srikanth::srikanthmart
