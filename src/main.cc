#include <drogon/drogon.h>
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <memory>
#include <filesystem>
#include <stdexcept>
#include "util/Environment.h"

#include "repository/DatabaseConnection.h"
#include "repository/RepositoryFactory.h"
#include "service/MigrationService.h"
#include "service/AuthService.h"
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

int main(int argc, char* argv[]) {
    LoadLocalEnvironment();
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::info("Starting SudharshanMart C++20 Server (Owner: Sudharshan)...");
    try {

    // 1. Initialize SQLite Database Connection (Singleton)
    const std::string db_file = GetEnvironmentValue("DB_PATH", "sudharshanmart.db");
    DatabaseConnection& db_conn = DatabaseConnection::GetInstance();
    const std::filesystem::path database_path(db_file);
    if (!database_path.parent_path().empty()) {
        std::filesystem::create_directories(database_path.parent_path());
    }
    db_conn.Initialize(db_file);

    // 2. Run Database Migrations and apply initial seed data
    MigrationService migration_service(db_conn);
    const auto app_root = std::filesystem::current_path();
    migration_service.RunMigrations((app_root / "db/migrations").string(), (app_root / "db/seed.sql").string());

    // 3. Instantiate Repositories via Factory Pattern (Dependency Inversion)
    std::shared_ptr<IUserRepository> user_repo = RepositoryFactory::CreateUserRepository(db_conn);
    std::shared_ptr<IProductRepository> product_repo = RepositoryFactory::CreateProductRepository(db_conn);
    std::shared_ptr<ICartRepository> cart_repo = RepositoryFactory::CreateCartRepository(db_conn);
    std::shared_ptr<IOrderRepository> order_repo = RepositoryFactory::CreateOrderRepository(db_conn);
    std::shared_ptr<IReviewRepository> review_repo = RepositoryFactory::CreateReviewRepository(db_conn);

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
        .enableSession(3600) // Explicit 1-hour idle session timeout
        .setClientMaxBodySize(10 * 1024 * 1024);

    spdlog::info("SudharshanMart is ready! Serving web frontend and API at http://localhost:{}", port);
    drogon::app().run();

    return 0;
    } catch (const std::exception& error) {
        spdlog::critical("Application startup/runtime failure: {}", error.what());
        return 1;
    }
}
