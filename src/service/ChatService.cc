#include "service/ChatService.h"
#include "util/ValidationUtil.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

ChatResponseDto ChatService::ProcessMessage(const std::string& session_id, const std::string& message) {
    // 1. Input validation & length cap
    ValidationUtil::ValidateNonEmpty(message, "message", "Message cannot be empty");
    if (message.length() > 500) {
        throw ValidationException("Message exceeds maximum allowed length of 500 characters", "message");
    }

    std::string safe_session_id = session_id.empty() ? "anonymous_session" : session_id;

    // 2. Thread-safe rate limit check (10 messages per minute)
    {
        std::lock_guard<std::mutex> lock(session_mutex_);
        if (!CheckRateLimit(safe_session_id)) {
            throw AppException("RATE_LIMIT_EXCEEDED", "Rate limit exceeded. Maximum 10 messages per minute allowed.", drogon::k429TooManyRequests);
        }

        // 3. Check repeated identical questions in session cache
        auto it = session_cache_.find(safe_session_id);
        if (it != session_cache_.end()) {
            auto cache_hit = it->second.find(message);
            if (cache_hit != it->second.end()) {
                spdlog::info("Returning cached AI response for session: {}", safe_session_id);
                ChatResponseDto dto;
                dto.reply = cache_hit->second;
                return dto;
            }
        }
    }

    // 4. Generate reply via provider
    std::string reply = provider_->GetReply(message, "SrikanthMart catalog");

    // 5. Cache response in session
    {
        std::lock_guard<std::mutex> lock(session_mutex_);
        session_cache_[safe_session_id][message] = reply;
    }

    ChatResponseDto dto;
    dto.reply = reply;
    return dto;
}

bool ChatService::CheckRateLimit(const std::string& session_id) {
    auto now = std::chrono::steady_clock::now();
    auto& timestamps = rate_limit_map_[session_id];

    // Purge timestamps older than 60 seconds
    while (!timestamps.empty() && std::chrono::duration_cast<std::chrono::seconds>(now - timestamps.front()).count() > 60) {
        timestamps.pop_front();
    }

    if (timestamps.size() >= 10) {
        return false;
    }

    timestamps.push_back(now);
    return true;
}

} // namespace srikanth::srikanthmart
