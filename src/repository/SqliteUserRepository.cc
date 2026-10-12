#include "repository/SqliteUserRepository.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace sudharshan::sudharshanmart {

std::optional<User> SqliteUserRepository::FindById(int64_t id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT id, name, email, password_hash, role, created_at FROM users WHERE id = ?;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, id);

    if (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int64(stmt.Get(), 0);
        u.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 1));
        u.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 2));
        u.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 3));
        u.role = StringToRole(reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 4)));
        u.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 5));
        return u;
    }
    return std::nullopt;
}

std::optional<User> SqliteUserRepository::FindByEmail(const std::string& email) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT id, name, email, password_hash, role, created_at FROM users WHERE lower(email) = lower(?);";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_text(stmt.Get(), 1, email.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int64(stmt.Get(), 0);
        u.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 1));
        u.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 2));
        u.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 3));
        u.role = StringToRole(reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 4)));
        u.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 5));
        return u;
    }
    return std::nullopt;
}

User SqliteUserRepository::CreateUser(const User& user) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "INSERT INTO users (name, email, password_hash, role) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }
    StatementRAII stmt(stmt_raw);

    sqlite3_bind_text(stmt.Get(), 1, user.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.Get(), 2, user.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.Get(), 3, user.password_hash.c_str(), -1, SQLITE_TRANSIENT);
    std::string role_str = RoleToString(user.role);
    sqlite3_bind_text(stmt.Get(), 4, role_str.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt.Get());
    if (rc == SQLITE_CONSTRAINT) {
        throw ConflictException("A user with this email address already exists");
    } else if (rc != SQLITE_DONE) {
        throw InternalServerException(std::string("Failed to insert user: ") + sqlite3_errmsg(db));
    }

    User created = user;
    created.id = sqlite3_last_insert_rowid(db);
    return created;
}

void SqliteUserRepository::UpdatePassword(int64_t id, const std::string& password_hash) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "UPDATE users SET password_hash = ? WHERE id = ? AND role = 'ADMIN';", -1, &stmt, nullptr) != SQLITE_OK)
        throw InternalServerException("Failed to prepare administrator password update");
    StatementRAII statement(stmt);
    sqlite3_bind_text(statement.Get(), 1, password_hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(statement.Get(), 2, id);
    if (sqlite3_step(statement.Get()) != SQLITE_DONE || sqlite3_changes(db) != 1)
        throw ConflictException("Administrator account changed during recovery; no password was updated");
}

std::vector<User> SqliteUserRepository::FindAll() {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT id, name, email, password_hash, role, created_at FROM users ORDER BY id ASC;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException(std::string("Failed to prepare statement: ") + sqlite3_errmsg(db));
    }
    StatementRAII stmt(stmt_raw);

    std::vector<User> list;
    while (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int64(stmt.Get(), 0);
        u.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 1));
        u.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 2));
        u.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 3));
        u.role = StringToRole(reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 4)));
        u.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 5));
        list.push_back(u);
    }
    return list;
}

} // namespace sudharshan::sudharshanmart
