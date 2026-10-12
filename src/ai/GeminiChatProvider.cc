#include "ai/GeminiChatProvider.h"
#include <cstdlib>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    auto* s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), total);
    return total;
}

GeminiChatProvider::GeminiChatProvider(std::string api_key)
    : api_key_(std::move(api_key)) {
    if (api_key_.empty()) {
        const char* env_key = std::getenv("GEMINI_API_KEY");
        if (env_key) {
            api_key_ = env_key;
        }
    }
}

std::string GeminiChatProvider::GetReply(const std::string& user_message,
                                         const std::string& /*context*/) {
    if (api_key_.empty()) {
        // Degraded fallback when API key is not configured in environment
        return "SrikanthMart AI Assistant (Degraded mode): Our live AI connection is offline, but I can still assist with store policies, orders, shipping, and product catalog inquiries!";
    }

    // Fixed server-side prompt template restricting scope to marketplace domain
    std::string system_prompt =
        "You are the intelligent customer service AI for 'SrikanthMart', a native C++ multi-seller e-commerce marketplace founded by Srikanth. "
        "Strict rule: You only answer inquiries concerning products, sellers, orders, shipping, returns, cart, checkout, or marketplace platform features. "
        "Decline questions unrelated to shopping or SrikanthMart politely. Be helpful, concise, and professional.";

    nlohmann::json payload;
    payload["contents"] = nlohmann::json::array({
        {
            {"role", "user"},
            {"parts", nlohmann::json::array({
                {{"text", system_prompt + "\n\nUser Question: " + user_message}}
            })}
        }
    });

    std::string json_body = payload.dump();
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + api_key_;

    CURL* curl = curl_easy_init();
    if (!curl) {
        return "SrikanthMart AI: Temporary network issue. Please ask again in a moment.";
    }

    std::string response_buffer;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6L); // 6s timeout guardrail

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || http_code != 200) {
        spdlog::warn("Gemini API call failed with code {} (curl code {}). Serving degraded fallback response.", http_code, static_cast<int>(res));
        return "SrikanthMart Assistant: Our AI engine is experiencing high load. You can browse our catalog, check your cart, or track shipments directly in your dashboard.";
    }

    try {
        auto resp_json = nlohmann::json::parse(response_buffer);
        if (resp_json.contains("candidates") && !resp_json["candidates"].empty()) {
            auto candidate = resp_json["candidates"][0];
            if (candidate.contains("content") && candidate["content"].contains("parts")) {
                std::string reply = candidate["content"]["parts"][0]["text"].get<std::string>();
                return reply;
            }
        }
    } catch (...) {
        // Fallback on JSON parse error
    }

    return "SrikanthMart Assistant: Thank you for asking. Our team is always here to help you shop seamlessly!";
}

} // namespace srikanth::srikanthmart
