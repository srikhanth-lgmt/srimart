#pragma once

#include <string>

namespace srikanth::srikanthmart {

/**
 * @brief Argon2id password hashing utility via libsodium (crypto_pwhash_str).
 */
class PasswordUtil {
public:
    static void Initialize();

    [[nodiscard]] static std::string HashPassword(const std::string& plain_password);

    [[nodiscard]] static bool VerifyPassword(const std::string& plain_password, const std::string& hashed_password);
};

} // namespace srikanth::srikanthmart
