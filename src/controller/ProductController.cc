#include "controller/ProductController.h"
#include "util/JsonUtil.h"

namespace sudharshan::sudharshanmart {

std::shared_ptr<ProductService> ProductController::product_service_ = nullptr;

void ProductController::ListProducts(const drogon::HttpRequestPtr& req,
                                     std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        std::string q = req->getParameter("q");
        std::string category = req->getParameter("category");

        auto products = product_service_->GetProducts(q, category);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& p : products) {
            arr.push_back(p.ToJson());
        }
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(arr));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void ProductController::GetProduct(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                   int64_t product_id) {
    std::string request_id = GetRequestId(req);
    try {
        auto p = product_service_->GetProductById(product_id);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(p.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void ProductController::CreateProduct(const drogon::HttpRequestPtr& req,
                                      std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required to create products");
        }
        std::string role = GetSessionUserRole(req);
        if (role != "SELLER") {
            throw ForbiddenException("Only sellers can list products");
        }

        auto json = JsonUtil::ParseRequestBody(req);
        CreateProductRequestDto dto;
        dto.name = json.value("name", "");
        dto.description = json.value("description", "");
        dto.price_cents = json.value("price_cents", 0LL);
        dto.stock_qty = json.value("stock_qty", 0);
        dto.category = json.value("category", "");
        dto.image_url = json.value("image_url", "");

        auto created = product_service_->CreateProduct(user_id_opt.value(), dto);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(created.ToJson()), drogon::k201Created);
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void ProductController::UpdateProduct(const drogon::HttpRequestPtr& req,
                                      std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                      int64_t product_id) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        if (GetSessionUserRole(req) != "SELLER") throw ForbiddenException("Seller account required");
        auto json = JsonUtil::ParseRequestBody(req);
        UpdateProductRequestDto dto;
        dto.name = json.value("name", "");
        dto.description = json.value("description", "");
        dto.price_cents = json.value("price_cents", 0LL);
        dto.stock_qty = json.value("stock_qty", 0);
        dto.category = json.value("category", "");
        dto.image_url = json.value("image_url", "");

        auto updated = product_service_->UpdateProduct(user_id_opt.value(), product_id, dto);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(updated.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void ProductController::DeleteProduct(const drogon::HttpRequestPtr& req,
                                      std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                      int64_t product_id) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        std::string role = GetSessionUserRole(req);
        if (role != "SELLER" && role != "ADMIN") throw ForbiddenException("Seller account required");
        bool is_admin = (role == "ADMIN");

        product_service_->DeleteProduct(user_id_opt.value(), is_admin, product_id);
        nlohmann::json data;
        data["message"] = "Product deleted successfully";
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(data));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

void ProductController::GetSellerProducts(const drogon::HttpRequestPtr& req,
                                          std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto user_id_opt = GetSessionUserId(req);
        if (!user_id_opt.has_value()) {
            throw UnauthorizedException("Session required");
        }
        if (GetSessionUserRole(req) != "SELLER") throw ForbiddenException("Seller account required");
        auto products = product_service_->GetProductsBySeller(user_id_opt.value());
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& p : products) {
            arr.push_back(p.ToJson());
        }
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(arr));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

} // namespace sudharshan::sudharshanmart
