#include "service/AdminSeedService.h"
#include "util/Environment.h"
#include "util/PasswordUtil.h"
#include "util/ValidationUtil.h"
#include "exception/AppException.h"
#include <algorithm>
#include <cctype>

namespace sudharshan::sudharshanmart {

void SeedAdminFromEnvironment(const std::shared_ptr<IUserRepository>& users, bool reset_existing) {
    std::string email = GetEnvironmentValue("ADMIN_EMAIL", "");
    const std::string password = GetEnvironmentValue("ADMIN_PASSWORD", "");
    if (email.empty() || password.empty()) throw ValidationException("ADMIN_EMAIL and ADMIN_PASSWORD must be set for admin seeding");
    std::transform(email.begin(), email.end(), email.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    ValidationUtil::ValidateEmail(email);
    ValidationUtil::ValidatePassword(password);

    const auto existing = users->FindByEmail(email);
    if (existing) {
        if (existing->role != UserRole::kAdmin) throw ConflictException("Configured admin email belongs to a non-admin account; no role changes were made");
        if (reset_existing) {
            users->UpdatePassword(existing->id, PasswordUtil::HashPassword(password));
            return;
        }
        if (!PasswordUtil::VerifyPassword(password, existing->password_hash))
            throw ConflictException("An administrator already exists with different credentials; use --reset-admin-password explicitly to rotate them");
        return;
    }

    User admin;
    admin.name = GetEnvironmentValue("ADMIN_NAME", "SudharsanMart Administrator");
    ValidationUtil::ValidateNonEmpty(admin.name, "ADMIN_NAME", "Admin name is required");
    admin.email = email;
    admin.password_hash = PasswordUtil::HashPassword(password);
    admin.role = UserRole::kAdmin;
    users->CreateUser(admin);
}

}
