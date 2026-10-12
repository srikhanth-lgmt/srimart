#pragma once

#include "ai/ChatProvider.h"
#include <string>

namespace srikanth::srikanthmart {

class GeminiChatProvider : public ChatProvider {
public:
    explicit GeminiChatProvider(std::string api_key = "");

    std::string GetReply(const std::string& user_message,
                         const std::string& context) override;

private:
    std::string api_key_;
};

} // namespace srikanth::srikanthmart
