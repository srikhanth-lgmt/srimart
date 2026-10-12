#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <deque>
#include <chrono>
#include <mutex>
#include "ai/ChatProvider.h"
#include "dto/ChatDto.h"

namespace srikanth::srikanthmart {

/**
 * @brief Chat Service managing AI interactions, session rate limits, and in-memory query caching.
 */
class ChatService {
public:
    explicit ChatService(std::shared_ptr<ChatProvider> provider)
        : provider_(std::move(provider)) {}

    /**
     * @brief Process a user query through AI provider with rate limiting and cache checks.
     * @param session_id Unique Drogon session identifier.
     * @param message User's input question.
     * @return ChatResponseDto containing reply.
     */
    ChatResponseDto ProcessMessage(const std::string& session_id, const std::string& message);

private:
    std::shared_ptr<ChatProvider> provider_;

    // Mutex guarding rate limit and cache maps
    std::mutex session_mutex_;

    // Per-session rate limiting: timestamps of requests within last minute
    std::unordered_map<std::string, std::deque<std::chrono::steady_clock::time_point>> rate_limit_map_;

    // Per-session question cache: session_id -> { message -> cached_reply }
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> session_cache_;

    bool CheckRateLimit(const std::string& session_id);
};

} // namespace srikanth::srikanthmart
