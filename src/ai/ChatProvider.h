#pragma once

#include <string>

namespace srikanth::srikanthmart {

/**
 * @brief Abstract AI Chat provider interface.
 */
class ChatProvider {
public:
    virtual ~ChatProvider() = default;

    /**
     * @brief Generates an AI response for customer inquiry.
     * @param user_message Raw query from buyer or seller.
     * @param context Domain context including marketplace product catalog and policies.
     * @return Formatted AI reply string.
     */
    virtual std::string GetReply(const std::string& user_message,
                                 const std::string& context) = 0;
};

} // namespace srikanth::srikanthmart
