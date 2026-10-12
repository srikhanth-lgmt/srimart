#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "model/User.h"

namespace sudharshan::sudharshanmart {

/**
 * @brief User registration request DTO.
 */
struct RegisterUserRequestDto {
    std::string name;
    std::string email;
    std::string password;
    std::string role; // BUYER or SELLER
    bool accepted_terms{false};
};

/**
 * @brief User login request DTO.
 */
struct LoginRequestDto {
    std::string email;
    std::string password;
};

/**
 * @brief Safe User Response DTO (strictly excludes password_hash).
 */
class UserResponseDto {
public:
    int64_t id{0};
    std::string name;
    std::string email;
    std::string role;
    std::string created_at;

    [[nodiscard]] nlohmann::json ToJson() const {
        return nlohmann::json{
            {"id", id},
            {"name", name},
            {"email", email},
            {"role", role},
            {"created_at", created_at}
        };
    }

    /**
     * @brief Fluent Builder for UserResponseDto. Declared here, defined after
     * the class is complete (see below) so it may hold UserResponseDto by value.
     */
    class Builder;

    static UserResponseDto FromEntity(const User& user);
};

class UserResponseDto::Builder {
public:
    Builder& SetId(int64_t id) { dto_.id = id; return *this; }
    Builder& SetName(std::string name) { dto_.name = std::move(name); return *this; }
    Builder& SetEmail(std::string email) { dto_.email = std::move(email); return *this; }
    Builder& SetRole(std::string role) { dto_.role = std::move(role); return *this; }
    Builder& SetCreatedAt(std::string created_at) { dto_.created_at = std::move(created_at); return *this; }

    [[nodiscard]] UserResponseDto Build() const {
        return dto_;
    }
private:
    UserResponseDto dto_;
};

inline UserResponseDto UserResponseDto::FromEntity(const User& user) {
    return Builder()
        .SetId(user.id)
        .SetName(user.name)
        .SetEmail(user.email)
        .SetRole(RoleToString(user.role))
        .SetCreatedAt(user.created_at)
        .Build();
}

} // namespace sudharshan::sudharshanmart
