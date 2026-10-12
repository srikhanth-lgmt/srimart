#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include <drogon/drogon.h>

namespace srikanth::srikanthmart {

/**
 * @brief Standardized API response error descriptor.
 */
struct ApiError {
    std::string code;
    std::string message;
    std::optional<std::string> field;

    nlohmann::json ToJson() const {
        nlohmann::json j;
        j["code"] = code;
        j["message"] = message;
        if (field.has_value()) {
            j["field"] = field.value();
        } else {
            j["field"] = nullptr;
        }
        return j;
    }
};

/**
 * @brief Standardized API response envelope matching specification:
 * { "success": true, "data": { ... }, "error": null }
 * { "success": false, "data": null, "error": { "code": "...", "message": "...", "field": null } }
 */
template <typename T = nlohmann::json>
class ApiResponse {
public:
    static nlohmann::json Success(const T& data) {
        nlohmann::json j;
        j["success"] = true;
        j["data"] = data;
        j["error"] = nullptr;
        return j;
    }

    static nlohmann::json Error(const std::string& code, const std::string& message, const std::optional<std::string>& field = std::nullopt) {
        nlohmann::json j;
        j["success"] = false;
        j["data"] = nullptr;
        ApiError err{code, message, field};
        j["error"] = err.ToJson();
        return j;
    }

    /**
     * @brief Converts nlohmann::json to Drogon HttpResponsePtr.
     */
    static drogon::HttpResponsePtr ToHttpResponse(const nlohmann::json& json_obj, drogon::HttpStatusCode status_code = drogon::k200OK) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(status_code);
        resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        resp->setBody(json_obj.dump());
        return resp;
    }
};

} // namespace srikanth::srikanthmart
