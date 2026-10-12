#include "filter/CorsFilter.h"
#include <drogon/HttpResponse.h>

namespace srikanth::srikanthmart {

void CorsFilter::doFilter(const drogon::HttpRequestPtr& req,
                          drogon::FilterCallback&& fcb,
                          drogon::FilterChainCallback&& fccb) {
    if (req->method() == drogon::HttpMethod::Options) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Request-Id");
        resp->setStatusCode(drogon::k200OK);
        fcb(resp);
        return;
    }
    fccb();
}

} // namespace srikanth::srikanthmart
