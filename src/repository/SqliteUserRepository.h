#pragma once

#include "repository/IUserRepository.h"
#include "repository/DatabaseConnection.h"

namespace srikanth::srikanthmart {

class SqliteUserRepository : public IUserRepository {
public:
    explicit SqliteUserRepository(DatabaseConnection& db_conn) : db_conn_(db_conn) {}

    std::optional<User> FindById(int64_t id) override;
    std::optional<User> FindByEmail(const std::string& email) override;
    User CreateUser(const User& user) override;
    void UpdatePassword(int64_t id, const std::string& password_hash) override;
    std::vector<User> FindAll() override;

private:
    DatabaseConnection& db_conn_;
};

} // namespace srikanth::srikanthmart
