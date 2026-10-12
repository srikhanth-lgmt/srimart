#include "controller/ChatController.h"
#include "util/JsonUtil.h"

namespace srikanth::srikanthmart {

std::shared_ptr<ChatService> ChatController::chat_service_ = nullptr;

void ChatController::Chat(const drogon::HttpRequestPtr& req,
                          std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    try {
        auto json = JsonUtil::ParseRequestBody(req);
        std::string message = json.value("message", "");

        std::string session_id = "default_session";
        auto session = req->session();
        if (session) {
            session_id = session->sessionId();
        }

        auto result = chat_service_->ProcessMessage(session_id, message);
        auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(result.ToJson()));
        callback(resp);
    } catch (const std::exception& ex) {
        callback(GlobalErrorHandler::HandleException(ex, request_id));
    }
}

} // namespace srikanth::srikanthmart
