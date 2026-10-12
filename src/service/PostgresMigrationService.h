#pragma once

#include <string>
#include "repository/PostgresConnection.h"

namespace srikanth::srikanthmart {

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
    void VerifyRequiredTables();

private:
    PostgresConnection& db_conn_;
    void EnsureTrackingTable();
    bool IsMigrationApplied(const std::string& version);
    bool ApplySqlFile(const std::string& file_path, const std::string& version);
};

} // namespace srikanth::srikanthmart
