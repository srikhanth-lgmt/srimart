#pragma once

#include <sqlite3.h>
#include <string>
#include <memory>
#include <stdexcept>
#include <mutex>
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

/**
 * @brief RAII statement wrapper ensuring automatic sqlite3_finalize on scope exit.
 */
class StatementRAII {
public:
    explicit StatementRAII(sqlite3_stmt* stmt) noexcept : stmt_(stmt) {}
    ~StatementRAII() {
        if (stmt_) {
            sqlite3_finalize(stmt_);
        }
    }
    StatementRAII(const StatementRAII&) = delete;
    StatementRAII& operator=(const StatementRAII&) = delete;
    StatementRAII(StatementRAII&& other) noexcept : stmt_(other.stmt_) { other.stmt_ = nullptr; }

    [[nodiscard]] sqlite3_stmt* Get() const noexcept { return stmt_; }
    explicit operator bool() const noexcept { return stmt_ != nullptr; }

private:
    sqlite3_stmt* stmt_{nullptr};
};

/**
 * @brief RAII transaction wrapper for SQLite. Rollback unless committed.
 */
class SqliteTransaction {
public:
    explicit SqliteTransaction(sqlite3* db) : db_(db) {
        char* err_msg = nullptr;
        if (sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::string err = err_msg ? err_msg : "Unknown SQLite error";
            sqlite3_free(err_msg);
            throw std::runtime_error("Failed to begin transaction: " + err);
        }
    }

    ~SqliteTransaction() {
        if (!committed_ && db_) {
            sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        }
    }

    void Commit() {
        if (committed_) return;
        char* err_msg = nullptr;
        if (sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::string err = err_msg ? err_msg : "Unknown SQLite error";
            sqlite3_free(err_msg);
            throw std::runtime_error("Failed to commit transaction: " + err);
        }
        committed_ = true;
    }

private:
    sqlite3* db_{nullptr};
    bool committed_{false};
};

/**
 * @brief Thread-safe Database connection manager (Singleton / Pool).
 */
class DatabaseConnection {
public:
    static DatabaseConnection& GetInstance() {
        static DatabaseConnection instance;
        return instance;
    }

    void Initialize(const std::string& db_path = "srikanthmart.db") {
        std::lock_guard<std::mutex> lock(mutex_);
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
        db_path_ = db_path;
        if (sqlite3_open(db_path_.c_str(), &db_) != SQLITE_OK) {
            std::string err = db_ ? sqlite3_errmsg(db_) : "SQLite could not allocate a connection";
            if (db_) { sqlite3_close(db_); db_ = nullptr; }
            throw std::runtime_error("Failed to open SQLite database: " + err);
        }
        // Enable foreign keys and WAL mode for high performance
        char* err_msg = nullptr;
        if (sqlite3_exec(db_, "PRAGMA foreign_keys = ON; PRAGMA journal_mode = WAL;", nullptr, nullptr, &err_msg) != SQLITE_OK) {
            const std::string err = err_msg ? err_msg : sqlite3_errmsg(db_);
            sqlite3_free(err_msg);
            sqlite3_close(db_); db_ = nullptr;
            throw std::runtime_error("Failed to configure SQLite database: " + err);
        }
        spdlog::info("Database initialized with SQLite at: {}", db_path_);
    }

    [[nodiscard]] sqlite3* GetDb() const {
        return db_;
    }

    [[nodiscard]] std::mutex& GetMutex() {
        return mutex_;
    }

    ~DatabaseConnection() {
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

private:
    DatabaseConnection() = default;
    sqlite3* db_{nullptr};
    std::string db_path_{"srikanthmart.db"};
    std::mutex mutex_;
};

} // namespace srikanth::srikanthmart
