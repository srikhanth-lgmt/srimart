#pragma once

#include <drogon/plugins/Plugin.h>
#include "service/MigrationService.h"
#include "repository/DatabaseConnection.h"

namespace srikanth::srikanthmart {

/**
 * @brief Startup Plugin invoking MigrationService to guarantee migrations run on process boot.
 */
class MigrationPlugin : public drogon::Plugin<MigrationPlugin> {
public:
    MigrationPlugin() = default;

    void initAndStart(const Json::Value& config) override;
    void shutdown() override;
};

} // namespace srikanth::srikanthmart
