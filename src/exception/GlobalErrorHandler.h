#pragma once

#include <drogon/drogon.h>
#include <exception>
#include "exception/AppException.h"
#include "dto/ApiResponse.h"

namespace srikanth::srikanthmart {

class GlobalErrorHandler {
public:
    static drogon::HttpResponsePtr HandleException(const std::exception& ex, const std::string& request_id = "") {
        if (const auto* app_ex = dynamic_cast<const AppException*>(&ex)) {
            auto json = ApiResponse<>::Error(app_ex->GetCode(), app_ex->GetMessage(), app_ex->GetField());
            if (!request_id.empty()) {
                json["request_id"] = request_id;
            }
            return ApiResponse<>::ToHttpResponse(json, app_ex->GetStatusCode());
        }

        // Generic / unhandled exception: never leak internal stack traces or database info
        auto json = ApiResponse<>::Error("INTERNAL_ERROR", "An unexpected error occurred. Please try again later.", std::nullopt);
        if (!request_id.empty()) {
            json["request_id"] = request_id;
        }
        return ApiResponse<>::ToHttpResponse(json, drogon::k500InternalServerError);
    }
};

} // namespace srikanth::srikanthmart
