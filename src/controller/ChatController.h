#pragma once

#include <drogon/HttpController.h>
#include "controller/BaseController.h"
#include "service/ChatService.h"

namespace srikanth::srikanthmart {

class ChatController : public drogon::HttpController<ChatController>, public BaseController {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ChatController::Chat, "/api/v1/chat", drogon::Post, "srikanth::srikanthmart::LoggingFilter", "srikanth::srikanthmart::CorsFilter");
    METHOD_LIST_END

    void Chat(const drogon::HttpRequestPtr& req,
              std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    static void SetChatService(std::shared_ptr<ChatService> service) {
        chat_service_ = std::move(service);
    }

private:
    static std::shared_ptr<ChatService> chat_service_;
};

} // namespace srikanth::srikanthmart
