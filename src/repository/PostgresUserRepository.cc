#include "repository/PostgresUserRepository.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace sudharshan::sudharshanmart {

namespace {
constexpr const char* kSelectColumns =
    "id, name, email, password_hash, role, "
    "to_char(created_at, 'YYYY-MM-DD HH24:MI:SS') AS created_at";

User ReadUser(PGresult* res, int row) {
    User u;
    u.id = PgInt64(res, row, 0);
    u.name = PgText(res, row, 1);
    u.email = PgText(res, row, 2);
    u.password_hash = PgText(res, row, 3);
    u.role = StringToRole(PgText(res, row, 4));
    u.created_at = PgText(res, row, 5);
    return u;
}
} // namespace

std::optional<User> PostgresUserRepository::FindById(int64_t id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        std::string("SELECT ") + kSelectColumns + " FROM users WHERE id = $1;",
        { std::to_string(id) });
    if (!res.IsOk()) {
        throw InternalServerException(std::string("Failed to query user: ") + PQerrorMessage(conn));
    }
    if (PQntuples(res.Get()) > 0) {
        return ReadUser(res.Get(), 0);
    }
    return std::nullopt;
}

std::optional<User> PostgresUserRepository::FindByEmail(const std::string& email) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        std::string("SELECT ") + kSelectColumns + " FROM users WHERE email = $1;",
        { email });
    if (!res.IsOk()) {
        throw InternalServerException(std::string("Failed to query user: ") + PQerrorMessage(conn));
    }
    if (PQntuples(res.Get()) > 0) {
        return ReadUser(res.Get(), 0);
    }
    return std::nullopt;
}

User PostgresUserRepository::CreateUser(const User& user) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "INSERT INTO users (name, email, password_hash, role) VALUES ($1, $2, $3, $4) RETURNING id;",
        { user.name, user.email, user.password_hash, RoleToString(user.role) });
    if (!res.IsOk()) {
        if (PgIsUniqueViolation(res.Get())) {
            throw ConflictException("A user with this email address already exists");
        }
        throw InternalServerException(std::string("Failed to insert user: ") + PQerrorMessage(conn));
    }

    User created = user;
    created.id = PgInt64(res.Get(), 0, 0);
    return created;
}

std::vector<User> PostgresUserRepository::FindAll() {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        std::string("SELECT ") + kSelectColumns + " FROM users ORDER BY id ASC;", {});
    if (!res.IsOk()) {
        throw InternalServerException(std::string("Failed to query users: ") + PQerrorMessage(conn));
    }

    std::vector<User> list;
    const int rows = PQntuples(res.Get());
    for (int i = 0; i < rows; ++i) {
        list.push_back(ReadUser(res.Get(), i));
    }
    return list;
}

} // namespace sudharshan::sudharshanmart
