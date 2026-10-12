#pragma once

#include <string>
#include <cstdint>

namespace srikanth::srikanthmart {

enum class UserRole {
    kBuyer,
    kSeller,
    kAdmin
};

inline std::string RoleToString(UserRole role) {
    switch (role) {
        case UserRole::kBuyer: return "BUYER";
        case UserRole::kSeller: return "SELLER";
        case UserRole::kAdmin: return "ADMIN";
    }
    return "BUYER";
}

inline UserRole StringToRole(const std::string& str) {
    if (str == "SELLER") return UserRole::kSeller;
    if (str == "ADMIN") return UserRole::kAdmin;
    return UserRole::kBuyer;
}

/**
 * @brief User entity model.
 */
struct User {
    int64_t id{0};
    std::string name;
    std::string email;
    std::string password_hash;
    UserRole role{UserRole::kBuyer};
    std::string created_at;
};

} // namespace srikanth::srikanthmart
