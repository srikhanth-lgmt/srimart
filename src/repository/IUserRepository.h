#pragma once

#include <optional>
#include <vector>
#include <string>
#include <memory>
#include "model/User.h"

namespace sudharshan::sudharshanmart {

/**
 * @brief Abstract repository interface for User data access.
 * Pure virtual interface adhering to Dependency Inversion Principle.
 */
class IUserRepository {
public:
    virtual ~IUserRepository() = default;

    /**
     * @brief Find user by primary key ID.
     */
    virtual std::optional<User> FindById(int64_t id) = 0;

    /**
     * @brief Find user by unique email address.
     */
    virtual std::optional<User> FindByEmail(const std::string& email) = 0;

    /**
     * @brief Persist a newly registered user.
     */
    virtual User CreateUser(const User& user) = 0;

    virtual void UpdatePassword(int64_t id, const std::string& password_hash) = 0;

    /**
     * @brief List all users for administration.
     */
    virtual std::vector<User> FindAll() = 0;
};

} // namespace sudharshan::sudharshanmart
