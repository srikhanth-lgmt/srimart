#pragma once

#include "ai/ChatProvider.h"
#include <unordered_map>

namespace srikanth::srikanthmart {

class MockChatProvider : public ChatProvider {
public:
    MockChatProvider();

    std::string GetReply(const std::string& user_message,
                         const std::string& context) override;

private:
    std::unordered_map<std::string, std::string> faq_db_;
};

} // namespace srikanth::srikanthmart
