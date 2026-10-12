#pragma once

#include <libpq-fe.h>
#include <string>
#include <memory>
#include <stdexcept>
#include <mutex>
#include <vector>
#include <optional>
#include <cstdint>
#include <cstdlib>
#include <spdlog/spdlog.h>

namespace sudharshan::sudharshanmart {

struct PostgresDatabaseInfo {
    std::string host;
    std::string port;
    std::string database;
    std::string schema;
    int public_table_count{0};
};

/**
 * @brief RAII wrapper around a libpq result, releasing it on scope exit.
 */
class PgResult {
public:
    explicit PgResult(PGresult* res) noexcept : res_(res) {}
    ~PgResult() {
        if (res_) {
            PQclear(res_);
        }
    }
    PgResult(const PgResult&) = delete;
    PgResult& operator=(const PgResult&) = delete;
    PgResult(PgResult&& other) noexcept : res_(other.res_) { other.res_ = nullptr; }
    PgResult& operator=(PgResult&& other) noexcept {
        if (this != &other) {
            if (res_) { PQclear(res_); }
            res_ = other.res_;
            other.res_ = nullptr;
        }
        return *this;
    }

    [[nodiscard]] PGresult* Get() const noexcept { return res_; }
    explicit operator bool() const noexcept { return res_ != nullptr; }
    [[nodiscard]] ExecStatusType Status() const noexcept { return res_ ? PQresultStatus(res_) : PGRES_FATAL_ERROR; }
    [[nodiscard]] bool IsOk() const noexcept {
        return res_ && (PQresultStatus(res_) == PGRES_TUPLES_OK || PQresultStatus(res_) == PGRES_COMMAND_OK);
    }

private:
    PGresult* res_{nullptr};
};

/**
 * @brief RAII transaction wrapper for PostgreSQL. Rollback unless committed.
 */
class PgTransaction {
public:
    explicit PgTransaction(PGconn* conn) : conn_(conn) {
        PgResult res(PQexec(conn_, "BEGIN;"));
        if (!res.IsOk()) {
            throw std::runtime_error(std::string("Failed to begin transaction: ") + PQerrorMessage(conn_));
        }
    }

    ~PgTransaction() {
        if (!committed_ && conn_) {
            PgResult res(PQexec(conn_, "ROLLBACK;"));
            (void)res;
        }
    }

    PgTransaction(const PgTransaction&) = delete;
    PgTransaction& operator=(const PgTransaction&) = delete;

    void Commit() {
        if (committed_) return;
        PgResult res(PQexec(conn_, "COMMIT;"));
        if (!res.IsOk()) {
            throw std::runtime_error(std::string("Failed to commit transaction: ") + PQerrorMessage(conn_));
        }
        committed_ = true;
    }

private:
    PGconn* conn_{nullptr};
    bool committed_{false};
};

/**
 * @brief Thread-safe PostgreSQL connection manager (Singleton).
 *
 * Uses libpq's synchronous API; every repository operation holds the shared
 * mutex so a single connection can be safely reused across Drogon worker
 * threads. This mirrors the SQLite connection strategy used elsewhere.
 */
class PostgresConnection {
public:
    static PostgresConnection& GetInstance() {
        static PostgresConnection instance;
        return instance;
    }

    void Initialize(const std::string& conninfo) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (conn_) {
            PQfinish(conn_);
            conn_ = nullptr;
        }
        conn_ = PQconnectdb(conninfo.c_str());
        if (!conn_ || PQstatus(conn_) != CONNECTION_OK) {
            std::string err = conn_ ? PQerrorMessage(conn_) : "libpq could not allocate a connection";
            if (conn_) { PQfinish(conn_); conn_ = nullptr; }
            throw std::runtime_error("Failed to connect to PostgreSQL: " + err);
        }
        // Keep all application DDL and queries in the schema operators inspect
        // in Neon Console, independent of a role-specific search_path.
        PgResult search_path(PQexec(conn_, "SET search_path TO public;"));
        if (!search_path.IsOk()) {
            const std::string err = PQerrorMessage(conn_);
            PQfinish(conn_);
            conn_ = nullptr;
            throw std::runtime_error("Failed to select PostgreSQL public schema: " + err);
        }
        const auto info = GetDatabaseInfoLocked();
        spdlog::info("Database initialized: PostgreSQL host={} port={} database={} schema={} public_tables={}",
                     info.host, info.port, info.database, info.schema, info.public_table_count);
    }

    [[nodiscard]] PGconn* Get() const { return conn_; }
    [[nodiscard]] std::mutex& GetMutex() { return mutex_; }
    [[nodiscard]] bool IsInitialized() const { return conn_ != nullptr; }

    /** Return connection target details only; never includes credentials. */
    PostgresDatabaseInfo GetDatabaseInfo() {
        std::lock_guard<std::mutex> lock(mutex_);
        return GetDatabaseInfoLocked();
    }

    /** @brief Lightweight liveness probe for the health endpoint. */
    bool Ping() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!conn_) return false;
        PgResult res(PQexec(conn_, "SELECT 1;"));
        return res.IsOk();
    }

    ~PostgresConnection() {
        if (conn_) {
            PQfinish(conn_);
            conn_ = nullptr;
        }
    }

private:
    PostgresDatabaseInfo GetDatabaseInfoLocked() {
        if (!conn_ || PQstatus(conn_) != CONNECTION_OK) {
            throw std::runtime_error("PostgreSQL connection is not available");
        }
        PgResult res(PQexec(conn_,
            "SELECT current_database(), current_schema(), "
            "(SELECT COUNT(*)::int FROM pg_catalog.pg_tables WHERE schemaname = 'public');"));
        if (!res.IsOk() || PQntuples(res.Get()) != 1) {
            throw std::runtime_error("Could not read PostgreSQL database/schema diagnostics");
        }
        return {
            PQhost(conn_) ? PQhost(conn_) : "?",
            PQport(conn_) ? PQport(conn_) : "?",
            PQgetvalue(res.Get(), 0, 0),
            PQgetvalue(res.Get(), 0, 1),
            static_cast<int>(std::strtol(PQgetvalue(res.Get(), 0, 2), nullptr, 10))
        };
    }

    PostgresConnection() = default;
    PGconn* conn_{nullptr};
    std::mutex mutex_;
};

/**
 * @brief Execute a parameterized SQL statement using positional ($1, $2, ...) placeholders.
 * A std::nullopt parameter is sent as SQL NULL.
 */
inline PgResult PgExecParams(PGconn* conn,
                             const std::string& sql,
                             const std::vector<std::optional<std::string>>& params) {
    std::vector<const char*> values(params.size(), nullptr);
    for (std::size_t i = 0; i < params.size(); ++i) {
        if (params[i].has_value()) {
            values[i] = params[i]->c_str();
        }
    }
    PGresult* raw = PQexecParams(conn, sql.c_str(),
                                 static_cast<int>(params.size()), nullptr,
                                 values.empty() ? nullptr : values.data(),
                                 nullptr, nullptr, 0);
    return PgResult(raw);
}

inline std::string PgSqlState(PGresult* res) {
    const char* state = res ? PQresultErrorField(res, PG_DIAG_SQLSTATE) : nullptr;
    return state ? state : "";
}

inline bool PgIsUniqueViolation(PGresult* res) {
    return PgSqlState(res) == "23505";
}

inline bool PgIsNull(PGresult* res, int row, int col) {
    return PQgetisnull(res, row, col) == 1;
}

inline std::string PgText(PGresult* res, int row, int col) {
    return PgIsNull(res, row, col) ? std::string() : std::string(PQgetvalue(res, row, col));
}

inline int64_t PgInt64(PGresult* res, int row, int col) {
    if (PgIsNull(res, row, col)) return 0;
    return std::stoll(PQgetvalue(res, row, col));
}

inline int32_t PgInt32(PGresult* res, int row, int col) {
    if (PgIsNull(res, row, col)) return 0;
    return static_cast<int32_t>(std::stol(PQgetvalue(res, row, col)));
}

} // namespace sudharshan::sudharshanmart
