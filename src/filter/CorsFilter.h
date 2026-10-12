#pragma once

#include <drogon/HttpFilter.h>

namespace srikanth::srikanthmart {

class CorsFilter : public drogon::HttpFilter<CorsFilter> {
public:
    void doFilter(const drogon::HttpRequestPtr& req,
                  drogon::FilterCallback&& fcb,
                  drogon::FilterChainCallback&& fccb) override;
};

} // namespace srikanth::srikanthmart
