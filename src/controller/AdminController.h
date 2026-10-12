#pragma once

#include <drogon/HttpController.h>
#include "controller/BaseController.h"
#include "repository/IUserRepository.h"
#include "service/OrderService.h"
#include "service/ProductService.h"

namespace srikanth::srikanthmart {

class AdminController : public drogon::HttpController<AdminController>, public BaseController {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AdminController::GetAllUsers, "/api/v1/admin/users", drogon::Get, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::AuthFilter", "srikanth::srikanthmart::CorsFilter");
    ADD_METHOD_TO(AdminController::GetAllOrders, "/api/v1/admin/orders", drogon::Get, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::AuthFilter", "srikanth::srikanthmart::CorsFilter");
    ADD_METHOD_TO(AdminController::ModerateProduct, "/api/v1/admin/products/{1}", drogon::Delete, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::AuthFilter", "srikanth::srikanthmart::CorsFilter");
    METHOD_LIST_END

    void GetAllUsers(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void GetAllOrders(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void ModerateProduct(const drogon::HttpRequestPtr& req,
                         std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                         int64_t product_id);

    static void InitializeDependencies(std::shared_ptr<IUserRepository> user_repo,
                                       std::shared_ptr<OrderService> order_service,
                                       std::shared_ptr<ProductService> product_service) {
        user_repo_ = std::move(user_repo);
        order_service_ = std::move(order_service);
        product_service_ = std::move(product_service);
    }

private:
    static std::shared_ptr<IUserRepository> user_repo_;
    static std::shared_ptr<OrderService> order_service_;
    static std::shared_ptr<ProductService> product_service_;

    static void EnforceAdminRole(const drogon::HttpRequestPtr& req);
};

} // namespace srikanth::srikanthmart
