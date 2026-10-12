#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <drogon/drogon.h>
#include "exception/AppException.h"

namespace srikanth::srikanthmart {

class JsonUtil {
public:
    static nlohmann::json ParseRequestBody(const drogon::HttpRequestPtr& req) {
        if (!req || req->getBody().empty()) {
            throw ValidationException("Request body cannot be empty");
        }
        try {
            return nlohmann::json::parse(req->getBody());
        } catch (const nlohmann::json::parse_error& e) {
            throw ValidationException("Invalid JSON payload: " + std::string(e.what()));
        }
    }
};

} // namespace srikanth::srikanthmart
