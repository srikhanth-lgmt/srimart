#include "service/MigrationService.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace sudharshan::sudharshanmart {

void MigrationService::RunMigrations(const std::string& migrations_dir, const std::string& seed_file) {
    EnsureTrackingTable();

    std::vector<fs::path> migration_files;
    if (!fs::exists(migrations_dir) || !fs::is_directory(migrations_dir)) {
        throw std::runtime_error("Migration directory is missing: " + migrations_dir);
    }
    {
        for (const auto& entry : fs::directory_iterator(migrations_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".sql") {
                migration_files.push_back(entry.path());
            }
        }
    }

    // Sort numerically by file name
    std::sort(migration_files.begin(), migration_files.end());

    for (const auto& file_path : migration_files) {
        std::string version = file_path.filename().string();
        if (!IsMigrationApplied(version)) {
            spdlog::info("Applying database migration: {}", version);
            ApplySqlFile(file_path.string());
            RecordMigration(version);
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
        ApplySqlFile(seed_file);
        RecordMigration(seed_version);
    }
}

void MigrationService::EnsureTrackingTable() {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();
    const char* sql = "CREATE TABLE IF NOT EXISTS schema_migrations ("
                      "version VARCHAR(64) PRIMARY KEY, "
                      "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        const std::string err = err_msg ? err_msg : sqlite3_errmsg(db);
        sqlite3_free(err_msg);
        throw std::runtime_error("Failed to initialize migration tracking table: " + err);
    }
}

bool MigrationService::IsMigrationApplied(const std::string& version) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();
    const char* sql = "SELECT 1 FROM schema_migrations WHERE version = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to query migration status: " + std::string(sqlite3_errmsg(db)));
    }
    sqlite3_bind_text(stmt, 1, version.c_str(), -1, SQLITE_TRANSIENT);
    bool applied = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return applied;
}

void MigrationService::RecordMigration(const std::string& version) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();
    const char* sql = "INSERT INTO schema_migrations (version) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, version.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            const std::string err = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to record migration " + version + ": " + err);
        }
        sqlite3_finalize(stmt);
    } else {
        throw std::runtime_error("Failed to prepare migration record: " + std::string(sqlite3_errmsg(db)));
    }
}

void MigrationService::ApplySqlFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open required SQL file: " + file_path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sql_contents = buffer.str();

    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql_contents.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::string err = err_msg ? err_msg : "Unknown error";
        sqlite3_free(err_msg);
        throw std::runtime_error("Error executing " + file_path + ": " + err);
    }
}

} // namespace sudharshan::sudharshanmart
