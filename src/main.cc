#include <drogon/drogon.h>
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <memory>
#include <filesystem>
#include <stdexcept>
#include "util/Environment.h"

#include "repository/DatabaseConnection.h"
#include "repository/PostgresConnection.h"
#include "repository/RepositoryFactory.h"
#include "service/MigrationService.h"
#include "service/PostgresMigrationService.h"
#include "service/AuthService.h"
#include "service/AdminSeedService.h"
#include "service/ProductService.h"
#include "service/CartService.h"
#include "service/OrderService.h"
#include "service/ReviewService.h"
#include "service/ChatService.h"
#include "ai/ChatProviderFactory.h"

#include "controller/AuthController.h"
#include "controller/ProductController.h"
#include "controller/CartController.h"
#include "controller/OrderController.h"
#include "controller/ReviewController.h"
#include "controller/AdminController.h"
#include "controller/ChatController.h"

using namespace sudharshan::sudharshanmart;

namespace {
// A PostgreSQL backend is selected when DATABASE_URL/POSTGRES_URL is a libpq URI.
bool IsPostgresUrl(const std::string& url) {
    return url.rfind("postgres://", 0) == 0 || url.rfind("postgresql://", 0) == 0;
}
} // namespace

int main(int argc, char* argv[]) {
    LoadLocalEnvironment();
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::info("Starting SudharshanMart C++20 Server (Owner: Sudharshan)...");
    try {

    const auto app_root = std::filesystem::current_path();

    // 1. Select and initialize the database backend.
    //    DATABASE_URL (or POSTGRES_URL) => PostgreSQL (e.g. Neon);
    //    otherwise fall back to the local SQLite file at DB_PATH.
    std::string database_url = GetEnvironmentValue("DATABASE_URL", "");
    if (database_url.empty()) database_url = GetEnvironmentValue("POSTGRES_URL", "");
    const bool use_postgres = IsPostgresUrl(database_url);
    const bool require_postgres = GetEnvironmentValue("REQUIRE_POSTGRES", "false") == "true";
    const std::string command = argc > 1 ? argv[1] : "";
    if (argc > 2 || (!command.empty() && command != "--check-db" && command != "--migrate" &&
                     command != "--seed-admin" && command != "--reset-admin-password")) {
        throw std::runtime_error("Supported commands: --check-db, --migrate, --seed-admin, --reset-admin-password");
    }
    if (require_postgres && database_url.empty()) {
        throw std::runtime_error("DATABASE_URL is required when REQUIRE_POSTGRES=true; refusing to fall back to SQLite");
    }
    if (!database_url.empty() && !use_postgres) {
        throw std::runtime_error("DATABASE_URL/POSTGRES_URL must use the postgres:// or postgresql:// scheme");
    }

    std::shared_ptr<IUserRepository> user_repo;
    std::shared_ptr<IProductRepository> product_repo;
    std::shared_ptr<ICartRepository> cart_repo;
    std::shared_ptr<IOrderRepository> order_repo;
    std::shared_ptr<IReviewRepository> review_repo;

    if (use_postgres) {
        spdlog::info("DATABASE_URL detected; using PostgreSQL backend");
        PostgresConnection& pg_conn = PostgresConnection::GetInstance();
        pg_conn.Initialize(database_url);
        if (command == "--check-db") {
            const auto info = pg_conn.GetDatabaseInfo();
            spdlog::info("Database diagnostic succeeded: backend=postgres host={} port={} database={} schema={} public_tables={}",
                         info.host, info.port, info.database, info.schema, info.public_table_count);
            return 0;
        }

        // 2. Run PostgreSQL migrations and seed data.
        PostgresMigrationService pg_migrations(pg_conn);
        pg_migrations.RunMigrations((app_root / "db/postgres/migrations").string(),
                                    (app_root / "db/postgres/seed.sql").string());

        user_repo = RepositoryFactory::CreateUserRepository(pg_conn);
        product_repo = RepositoryFactory::CreateProductRepository(pg_conn);
        cart_repo = RepositoryFactory::CreateCartRepository(pg_conn);
        order_repo = RepositoryFactory::CreateOrderRepository(pg_conn);
        review_repo = RepositoryFactory::CreateReviewRepository(pg_conn);
    } else {
        // 1. Initialize SQLite Database Connection (Singleton).
        const std::string db_file = GetEnvironmentValue("DB_PATH", "sudharshanmart.db");
        DatabaseConnection& db_conn = DatabaseConnection::GetInstance();
        const std::filesystem::path database_path(db_file);
        if (!database_path.parent_path().empty()) {
            std::filesystem::create_directories(database_path.parent_path());
        }
        db_conn.Initialize(db_file);
        if (command == "--check-db") {
            spdlog::info("Database diagnostic succeeded: backend=sqlite path={}", db_file);
            return 0;
        }

        // 2. Run SQLite migrations and seed data.
        MigrationService migration_service(db_conn);
        migration_service.RunMigrations((app_root / "db/migrations").string(), (app_root / "db/seed.sql").string());

        user_repo = RepositoryFactory::CreateUserRepository(db_conn);
        product_repo = RepositoryFactory::CreateProductRepository(db_conn);
        cart_repo = RepositoryFactory::CreateCartRepository(db_conn);
        order_repo = RepositoryFactory::CreateOrderRepository(db_conn);
        review_repo = RepositoryFactory::CreateReviewRepository(db_conn);
    }

    if (command == "--migrate") {
        spdlog::info("Database migrations completed and required schema verified");
        return 0;
    }

    if (command == "--seed-admin" || command == "--reset-admin-password") {
        const bool reset_existing = command == "--reset-admin-password";
        SeedAdminFromEnvironment(user_repo, reset_existing);
        spdlog::info("Administrator seed completed (no credentials logged)");
        return 0;
    }

    // 4. Instantiate Services (Pure C++ business rules, constructor injected)
    auto auth_service = std::make_shared<AuthService>(user_repo);
    auto product_service = std::make_shared<ProductService>(product_repo);
    auto cart_service = std::make_shared<CartService>(cart_repo, product_repo);
    auto order_service = std::make_shared<OrderService>(order_repo, cart_repo);
    auto review_service = std::make_shared<ReviewService>(review_repo, product_repo);

    // 5. Instantiate AI Chatbot Provider (Mock or Gemini via Factory)
    const std::string ai_provider_type = GetEnvironmentValue("AI_CHATBOT_PROVIDER", "mock");
    std::shared_ptr<ChatProvider> chat_provider = ChatProviderFactory::CreateProvider(ai_provider_type);
    auto chat_service = std::make_shared<ChatService>(chat_provider);

    // 6. Wire Dependencies into Controllers
    AuthController::SetAuthService(auth_service);
    ProductController::SetProductService(product_service);
    CartController::SetCartService(cart_service);
    OrderController::SetOrderService(order_service);
    ReviewController::SetReviewService(review_service);
    AdminController::InitializeDependencies(user_repo, order_service, product_service);
    ChatController::SetChatService(chat_service);

    // 7. Drogon Server Configuration
    const std::string port_str = GetEnvironmentValue("PORT", "8080");
    const long port_value = std::stol(port_str);
    if (port_value < 1 || port_value > 65535) {
        throw std::runtime_error("PORT must be an integer between 1 and 65535");
    }
    const auto port = static_cast<uint16_t>(port_value);

    spdlog::info("Configuring Drogon listener on 0.0.0.0:{}", port);
    drogon::app()
        .addListener("0.0.0.0", port)
        .setDocumentRoot((app_root / "public").string())
        .enableSession(3600, drogon::Cookie::SameSite::kLax, "SUDHARMART_SESSION") // 1-hour idle, same-site browser session
        .setClientMaxBodySize(10 * 1024 * 1024);

    spdlog::info("SudharshanMart is ready! Serving web frontend and API at http://localhost:{}", port);
    drogon::app().run();

    return 0;
    } catch (const std::exception& error) {
        spdlog::critical("Application startup/runtime failure: {}", error.what());
        return 1;
    }
}
