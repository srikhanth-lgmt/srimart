#pragma once

#include "repository/IUserRepository.h"
#include "repository/PostgresConnection.h"

namespace sudharshan::sudharshanmart {

class PostgresUserRepository : public IUserRepository {
public:
    explicit PostgresUserRepository(PostgresConnection& db_conn) : db_conn_(db_conn) {}

    std::optional<User> FindById(int64_t id) override;
    std::optional<User> FindByEmail(const std::string& email) override;
    User CreateUser(const User& user) override;
    std::vector<User> FindAll() override;

private:
    PostgresConnection& db_conn_;
};

} // namespace sudharshan::sudharshanmart
