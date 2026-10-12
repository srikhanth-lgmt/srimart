#pragma once

#include <drogon/HttpController.h>
#include <string>
#include <optional>
#include "dto/ApiResponse.h"
#include "exception/GlobalErrorHandler.h"

namespace sudharshan::sudharshanmart {

/**
 * @brief Common Base Controller providing session extraction and response helpers.
 * Implements the Front Controller base pattern requirement.
 */
class BaseController {
public:
    virtual ~BaseController() = default;

protected:
    static std::optional<int64_t> GetSessionUserId(const drogon::HttpRequestPtr& req) {
        auto session = req->session();
        if (session && session->find("user_id")) {
            try {
                return session->get<int64_t>("user_id");
            } catch (...) {}
        }
        return std::nullopt;
    }

    static std::string GetSessionUserRole(const drogon::HttpRequestPtr& req) {
        auto session = req->session();
        if (session && session->find("user_role")) {
            return session->get<std::string>("user_role");
        }
        return "BUYER";
    }

    static std::string GetRequestId(const drogon::HttpRequestPtr& req) {
        return req->getParameter("request_id");
    }
};

} // namespace sudharshan::sudharshanmart
