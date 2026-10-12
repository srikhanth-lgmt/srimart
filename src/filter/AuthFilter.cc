#include "filter/AuthFilter.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

void AuthFilter::doFilter(const drogon::HttpRequestPtr& req,
                          drogon::FilterCallback&& fcb,
                          drogon::FilterChainCallback&& fccb) {
    auto session = req->session();
    if (!session || !session->find("user_id")) {
        spdlog::warn("Unauthorized access blocked at {}", req->getPath());
        auto err_json = ApiResponse<>::Error("UNAUTHORIZED", "Active session required");
        auto resp = ApiResponse<>::ToHttpResponse(err_json, drogon::k401Unauthorized);
        fcb(resp);
        return;
    }

    fccb();
}

} // namespace srikanth::srikanthmart
