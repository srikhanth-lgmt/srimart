#include "plugin/MigrationPlugin.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

void MigrationPlugin::initAndStart(const Json::Value& config) {
    std::string migrations_dir = "db/migrations";
    std::string seed_file = "db/seed.sql";

    if (config.isMember("migrations_dir") && config["migrations_dir"].isString()) {
        migrations_dir = config["migrations_dir"].asString();
    }
    if (config.isMember("seed_file") && config["seed_file"].isString()) {
        seed_file = config["seed_file"].asString();
    }

    spdlog::info("MigrationPlugin running database migrations from: {}", migrations_dir);
    MigrationService migration_service(DatabaseConnection::GetInstance());
    migration_service.RunMigrations(migrations_dir, seed_file);
}

void MigrationPlugin::shutdown() {
    // No-op
}

} // namespace srikanth::srikanthmart
