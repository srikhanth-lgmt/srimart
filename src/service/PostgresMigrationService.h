#pragma once

#include <string>
#include "repository/PostgresConnection.h"

namespace sudharshan::sudharshanmart {

/**
 * @brief Hand-written PostgreSQL migration runner.
 * Mirrors MigrationService but targets a libpq connection and the PostgreSQL
 * dialect under db/postgres/.
 */
class PostgresMigrationService {
public:
    explicit PostgresMigrationService(PostgresConnection& db_conn) : db_conn_(db_conn) {}

    void RunMigrations(const std::string& migrations_dir = "db/postgres/migrations",
                      const std::string& seed_file = "db/postgres/seed.sql");

private:
    PostgresConnection& db_conn_;
    void EnsureTrackingTable();
    bool IsMigrationApplied(const std::string& version);
    void RecordMigration(const std::string& version);
    void ApplySqlFile(const std::string& file_path);
};

} // namespace sudharshan::sudharshanmart
