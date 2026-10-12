#pragma once

#include <drogon/HttpController.h>
#include "controller/BaseController.h"
#include "service/AuthService.h"

namespace srikanth::srikanthmart {

class AuthController : public drogon::HttpController<AuthController>, public BaseController {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AuthController::Register, "/api/v1/auth/register", drogon::Post, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::CorsFilter");
    ADD_METHOD_TO(AuthController::Login, "/api/v1/auth/login", drogon::Post, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::CorsFilter");
    ADD_METHOD_TO(AuthController::Logout, "/api/v1/auth/logout", drogon::Post, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::CorsFilter");
    ADD_METHOD_TO(AuthController::GetCurrentUser, "/api/v1/auth/me", drogon::Get, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::CorsFilter");
    METHOD_LIST_END

    void Register(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void Login(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void Logout(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void GetCurrentUser(const drogon::HttpRequestPtr& req,
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    static void SetAuthService(std::shared_ptr<AuthService> service) {
        auth_service_ = std::move(service);
    }

private:
    static std::shared_ptr<AuthService> auth_service_;
};

} // namespace srikanth::srikanthmart
