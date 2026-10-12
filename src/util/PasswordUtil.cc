#include "util/PasswordUtil.h"
#include <sodium.h>
#include <stdexcept>
#include <cstring>
#include <iostream>

namespace srikanth::srikanthmart {

void PasswordUtil::Initialize() {
    static bool initialized = false;
    if (!initialized) {
        if (sodium_init() < 0) {
            std::cerr << "Warning: sodium_init failed" << std::endl;
        }
        initialized = true;
    }
}

std::string PasswordUtil::HashPassword(const std::string& plain_password) {
    Initialize();
    char hashed[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(
            hashed,
            plain_password.c_str(),
            plain_password.length(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw std::runtime_error("Password hashing failed: memory or CPU limit exceeded");
    }
    return std::string(hashed);
}

bool PasswordUtil::VerifyPassword(const std::string& plain_password, const std::string& hashed_password) {
    Initialize();
    if (hashed_password.empty()) {
        return false;
    }
    if (crypto_pwhash_str_verify(
            hashed_password.c_str(),
            plain_password.c_str(),
            plain_password.length()) == 0) {
        return true;
    }
    return false;
}

} // namespace srikanth::srikanthmart
