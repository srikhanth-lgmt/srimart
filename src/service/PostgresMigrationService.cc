#include "service/PostgresMigrationService.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <stdexcept>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace sudharshan::sudharshanmart {

void PostgresMigrationService::RunMigrations(const std::string& migrations_dir, const std::string& seed_file) {
    EnsureTrackingTable();

    if (!fs::exists(migrations_dir) || !fs::is_directory(migrations_dir)) {
        throw std::runtime_error("Migration directory is missing: " + migrations_dir);
    }

    std::vector<fs::path> migration_files;
    for (const auto& entry : fs::directory_iterator(migrations_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".sql") {
            migration_files.push_back(entry.path());
        }
    }
    std::sort(migration_files.begin(), migration_files.end());

    for (const auto& file_path : migration_files) {
        std::string version = file_path.filename().string();
        if (!IsMigrationApplied(version)) {
            spdlog::info("Applying database migration: {}", version);
            ApplySqlFile(file_path.string(), version);
        } else {
            spdlog::debug("Migration {} already applied, skipping", version);
        }
    }

    constexpr const char* seed_version = "seed.sql";
    if (!fs::exists(seed_file)) {
        throw std::runtime_error("Seed SQL file is missing: " + seed_file);
    }
    if (!IsMigrationApplied(seed_version)) {
        spdlog::info("Applying seed data from: {}", seed_file);
        ApplySqlFile(seed_file, seed_version);
    }
    VerifyRequiredTables();
}

void PostgresMigrationService::VerifyRequiredTables() {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();
    PgResult res(PQexec(conn,
        "SELECT required.name FROM unnest(ARRAY["
        "'schema_migrations','users','products','orders','order_items','cart_items','wishlist_items','reviews'"
        "]) AS required(name) "
        "LEFT JOIN pg_catalog.pg_tables t ON t.schemaname = 'public' AND t.tablename = required.name "
        "WHERE t.tablename IS NULL ORDER BY required.name;"));
    if (!res.IsOk()) {
        throw std::runtime_error("Failed to verify required tables in PostgreSQL public schema");
    }
    if (PQntuples(res.Get()) > 0) {
        std::string missing;
        for (int row = 0; row < PQntuples(res.Get()); ++row) {
            if (!missing.empty()) missing += ", ";
            missing += PQgetvalue(res.Get(), row, 0);
        }
        throw std::runtime_error("PostgreSQL migration verification failed; missing public tables: " + missing);
    }
}

void PostgresMigrationService::EnsureTrackingTable() {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();
    PgResult res(PQexec(conn,
        "CREATE TABLE IF NOT EXISTS schema_migrations ("
        "version VARCHAR(64) PRIMARY KEY, "
        "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);"));
    if (!res.IsOk()) {
        throw std::runtime_error(std::string("Failed to initialize migration tracking table: ") + PQerrorMessage(conn));
    }
}

bool PostgresMigrationService::IsMigrationApplied(const std::string& version) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();
    PgResult res = PgExecParams(conn,
        "SELECT 1 FROM schema_migrations WHERE version = $1;",
        { version });
    if (!res.IsOk()) {
        throw std::runtime_error(std::string("Failed to query migration status: ") + PQerrorMessage(conn));
    }
    return PQntuples(res.Get()) > 0;
}

bool PostgresMigrationService::ApplySqlFile(const std::string& file_path, const std::string& version) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open required SQL file: " + file_path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sql_contents = buffer.str();

    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();
    PgTransaction transaction(conn);
    PgResult migration_lock(PQexec(conn, "SELECT pg_advisory_xact_lock(706245183);"));
    if (!migration_lock.IsOk()) {
        throw std::runtime_error("Could not acquire PostgreSQL migration lock");
    }
    // A concurrent instance may have applied this version while this runner
    // waited for the advisory lock. Check again inside the transaction.
    PgResult already_applied = PgExecParams(conn,
        "SELECT 1 FROM schema_migrations WHERE version = $1;", { version });
    if (!already_applied.IsOk()) {
        throw std::runtime_error("Failed to recheck PostgreSQL migration status");
    }
    if (PQntuples(already_applied.Get()) > 0) {
        transaction.Commit();
        return false;
    }
    PgResult res(PQexec(conn, sql_contents.c_str()));
    if (!res.IsOk()) {
        throw std::runtime_error("Error executing " + file_path + ": " + PQerrorMessage(conn));
    }
    PgResult record = PgExecParams(conn,
        "INSERT INTO schema_migrations (version) VALUES ($1);", { version });
    if (!record.IsOk()) {
        throw std::runtime_error("Failed to record PostgreSQL migration " + version);
    }
    transaction.Commit();
    return true;
}

} // namespace sudharshan::sudharshanmart
