#pragma once

#include <memory>
#include <string>
#include "repository/IUserRepository.h"
#include "dto/UserDto.h"

namespace srikanth::srikanthmart {

/**
 * @brief Authentication Service encapsulating user registration, password verification,
 * and role-based access orchestration.
 */
class AuthService {
public:
    /**
     * @brief Constructor with constructor-injected repository interface (Dependency Inversion).
     * @param user_repo Shared pointer to abstract user repository.
     */
    explicit AuthService(std::shared_ptr<IUserRepository> user_repo)
        : user_repo_(std::move(user_repo)) {}

    /**
     * @brief Registers a new user as BUYER or SELLER.
     * @param request Registration payload containing name, email, password, and role.
     * @return UserResponseDto strictly without sensitive credentials.
     * @throws ValidationException on invalid input.
     * @throws ConflictException if email is already taken.
     */
    UserResponseDto Register(const RegisterUserRequestDto& request);

    /**
     * @brief Authenticates a user with email and password.
     * @param request Login credentials.
     * @return UserResponseDto representing authenticated identity.
     * @throws UnauthorizedException on invalid credentials.
     */
    UserResponseDto Login(const LoginRequestDto& request);

    /**
     * @brief Fetch user details by ID.
     */
    UserResponseDto GetUserById(int64_t id);

private:
    std::shared_ptr<IUserRepository> user_repo_;
};

} // namespace srikanth::srikanthmart
