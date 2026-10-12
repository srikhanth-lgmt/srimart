#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace srikanth::srikanthmart {

struct ChatRequestDto {
    std::string message;
};

struct ChatResponseDto {
    std::string reply;

    [[nodiscard]] nlohmann::json ToJson() const {
        return nlohmann::json{
            {"reply", reply}
        };
    }
};

} // namespace srikanth::srikanthmart
