#pragma once

#include <string>
#include <vector>
#include "repository/DatabaseConnection.h"

namespace srikanth::srikanthmart {

/**
 * @brief Hand-written database migration runner.
 * Reads migrations in numeric order, verifies against schema_migrations table,
 * and applies pending migrations atomically.
 */
class MigrationService {
public:
    explicit MigrationService(DatabaseConnection& db_conn) : db_conn_(db_conn) {}

    /**
     * @brief Execute all pending migrations and optionally apply seed data.
     * @param migrations_dir Path to folder containing V1__..., V2__... sql files.
     * @param seed_file Path to seed.sql.
     */
    void RunMigrations(const std::string& migrations_dir = "db/migrations",
                      const std::string& seed_file = "db/seed.sql");

private:
    DatabaseConnection& db_conn_;
    void EnsureTrackingTable();
    bool IsMigrationApplied(const std::string& version);
    void RecordMigration(const std::string& version);
    void ApplySqlFile(const std::string& file_path);
};

} // namespace srikanth::srikanthmart
