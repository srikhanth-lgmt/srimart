#include <gtest/gtest.h>
#include "service/ChatService.h"
#include "ai/MockChatProvider.h"
#include "exception/AppException.h"

using namespace srikanth::srikanthmart;

TEST(ChatServiceTest, AnswersDomainInquiries) {
    auto mock_provider = std::make_shared<MockChatProvider>();
    ChatService chat_service(mock_provider);

    ChatResponseDto resp = chat_service.ProcessMessage("session_1", "What is the order status workflow?");
    EXPECT_FALSE(resp.reply.empty());
    EXPECT_NE(resp.reply.find("Pending -> Confirmed -> Shipped -> Delivered"), std::string::npos);
}

TEST(ChatServiceTest, EnforcesRateLimit) {
    auto mock_provider = std::make_shared<MockChatProvider>();
    ChatService chat_service(mock_provider);

    // Send 10 messages within same session
    for (int i = 0; i < 10; ++i) {
        EXPECT_NO_THROW(chat_service.ProcessMessage("rate_limited_session", "Question " + std::to_string(i)));
    }

    // 11th message must be throttled with rate limit exception
    EXPECT_THROW(chat_service.ProcessMessage("rate_limited_session", "Question 11"), AppException);
}

TEST(ChatServiceTest, RejectsExcessiveInputLength) {
    auto mock_provider = std::make_shared<MockChatProvider>();
    ChatService chat_service(mock_provider);

    std::string long_query(501, 'a');
    EXPECT_THROW(chat_service.ProcessMessage("session_len", long_query), ValidationException);
}
