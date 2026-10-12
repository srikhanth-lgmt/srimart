#pragma once

#include <drogon/HttpController.h>
#include "controller/BaseController.h"
#include "service/ProductService.h"

namespace srikanth::srikanthmart {

class ProductController : public drogon::HttpController<ProductController>, public BaseController {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ProductController::ListProducts, "/api/v1/products", drogon::Get, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::CorsFilter");
    ADD_METHOD_TO(ProductController::GetProduct, "/api/v1/products/{1}", drogon::Get, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::CorsFilter");
    ADD_METHOD_TO(ProductController::CreateProduct, "/api/v1/products", drogon::Post, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::AuthFilter", "srikanth::srikanthmart::CorsFilter");
    ADD_METHOD_TO(ProductController::UpdateProduct, "/api/v1/products/{1}", drogon::Put, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::AuthFilter", "srikanth::srikanthmart::CorsFilter");
    ADD_METHOD_TO(ProductController::DeleteProduct, "/api/v1/products/{1}", drogon::Delete, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::AuthFilter", "srikanth::srikanthmart::CorsFilter");
    ADD_METHOD_TO(ProductController::GetSellerProducts, "/api/v1/products/seller/mine", drogon::Get, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::AuthFilter", "srikanth::srikanthmart::CorsFilter");
    METHOD_LIST_END

    void ListProducts(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void GetProduct(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    int64_t product_id);

    void CreateProduct(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void UpdateProduct(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                       int64_t product_id);

    void DeleteProduct(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                       int64_t product_id);

    void GetSellerProducts(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    static void SetProductService(std::shared_ptr<ProductService> service) {
        product_service_ = std::move(service);
    }

private:
    static std::shared_ptr<ProductService> product_service_;
};

} // namespace srikanth::srikanthmart
