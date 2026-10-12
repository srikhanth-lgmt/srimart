#include "ai/ChatProviderFactory.h"
#include "ai/MockChatProvider.h"
#include "ai/GeminiChatProvider.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

std::unique_ptr<ChatProvider> ChatProviderFactory::CreateProvider(const std::string& provider_type,
                                                                 const std::string& api_key) {
    if (provider_type == "gemini") {
        spdlog::info("Configured AI Chat Provider: GeminiChatProvider");
        return std::make_unique<GeminiChatProvider>(api_key);
    }
    spdlog::info("Configured AI Chat Provider: MockChatProvider (canned FAQ)");
    return std::make_unique<MockChatProvider>();
}

} // namespace srikanth::srikanthmart
