#pragma once

#include "repository/IUserRepository.h"
#include <memory>

namespace sudharshan::sudharshanmart {

// Creates the initial administrator only when explicitly invoked with credentials.
void SeedAdminFromEnvironment(const std::shared_ptr<IUserRepository>& users, bool reset_existing = false);

}
