#include "service/AuthService.h"
#include "util/ValidationUtil.h"
#include "util/PasswordUtil.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cctype>

namespace sudharshan::sudharshanmart {

namespace {
std::string NormalizeEmail(std::string email) {
    email.erase(email.begin(), std::find_if(email.begin(), email.end(), [](unsigned char c) { return !std::isspace(c); }));
    email.erase(std::find_if(email.rbegin(), email.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), email.end());
    std::transform(email.begin(), email.end(), email.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return email;
}
}

UserResponseDto AuthService::Register(const RegisterUserRequestDto& request) {
    // 1. Validation at top of service method
    ValidationUtil::ValidateNonEmpty(request.name, "name", "Full name is required");
    const std::string email = NormalizeEmail(request.email);
    ValidationUtil::ValidateEmail(email);
    ValidationUtil::ValidatePassword(request.password);
    if (!request.accepted_terms) throw ValidationException("You must accept the terms and conditions", "accepted_terms");

    if (request.role != "BUYER" && request.role != "SELLER") {
        throw ValidationException("Registration role must be either BUYER or SELLER", "role");
    }

    // 2. Check if email already registered
    if (user_repo_->FindByEmail(email).has_value()) {
        throw ConflictException("An account with this email address already exists");
    }

    // 3. Argon2id password hashing
    std::string hashed_password = PasswordUtil::HashPassword(request.password);

    // 4. Construct entity
    User user;
    user.name = ValidationUtil::SanitizeHtml(request.name);
    user.email = email;
    user.password_hash = hashed_password;
    user.role = StringToRole(request.role);

    // 5. Persist through repository interface
    User created_user = user_repo_->CreateUser(user);

    spdlog::info("Successfully registered new user: {} (role: {})", created_user.email, request.role);
    return UserResponseDto::FromEntity(created_user);
}

UserResponseDto AuthService::Login(const LoginRequestDto& request) {
    // 1. Validation at top of service method
    const std::string email = NormalizeEmail(request.email);
    ValidationUtil::ValidateEmail(email);
    ValidationUtil::ValidateNonEmpty(request.password, "password", "Password cannot be empty");

    // 2. Query user by email
    auto user_opt = user_repo_->FindByEmail(email);
    if (!user_opt.has_value()) {
        throw UnauthorizedException("Invalid email or password");
    }

    const User& user = user_opt.value();

    // 3. Verify Argon2id password hash
    if (!PasswordUtil::VerifyPassword(request.password, user.password_hash)) {
        throw UnauthorizedException("Invalid email or password");
    }

    spdlog::info("User logged in successfully: {} (role: {})", user.email, RoleToString(user.role));
    return UserResponseDto::FromEntity(user);
}

UserResponseDto AuthService::GetUserById(int64_t id) {
    auto user_opt = user_repo_->FindById(id);
    if (!user_opt.has_value()) {
        throw NotFoundException("User not found");
    }
    return UserResponseDto::FromEntity(user_opt.value());
}

} // namespace sudharshan::sudharshanmart
