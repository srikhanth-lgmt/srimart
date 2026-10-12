#pragma once

#include <memory>
#include <string>
#include "ai/ChatProvider.h"

namespace srikanth::srikanthmart {

/**
 * @brief Factory for creating ChatProvider instances (Mock or Gemini) based on config.
 */
class ChatProviderFactory {
public:
    static std::unique_ptr<ChatProvider> CreateProvider(const std::string& provider_type,
                                                       const std::string& api_key = "");
};

} // namespace srikanth::srikanthmart
