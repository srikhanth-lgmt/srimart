#pragma once

#include <memory>
#include "repository/IUserRepository.h"
#include "repository/IProductRepository.h"
#include "repository/ICartRepository.h"
#include "repository/IOrderRepository.h"
#include "repository/IReviewRepository.h"
#include "repository/DatabaseConnection.h"
#include "repository/PostgresConnection.h"

namespace srikanth::srikanthmart {

/**
 * @brief Factory for instantiating repository implementations.
 * Satisfies the Factory Design Pattern requirement returning std::unique_ptr<Interface>.
 *
 * Overloads accept either a SQLite DatabaseConnection or a PostgreSQL
 * PostgresConnection, so the concrete backend is selected once at startup
 * (based on DATABASE_URL) while services remain backend-agnostic.
 */
class RepositoryFactory {
public:
    // SQLite-backed repositories
    static std::unique_ptr<IUserRepository> CreateUserRepository(DatabaseConnection& db_conn);
    static std::unique_ptr<IProductRepository> CreateProductRepository(DatabaseConnection& db_conn);
    static std::unique_ptr<ICartRepository> CreateCartRepository(DatabaseConnection& db_conn);
    static std::unique_ptr<IOrderRepository> CreateOrderRepository(DatabaseConnection& db_conn);
    static std::unique_ptr<IReviewRepository> CreateReviewRepository(DatabaseConnection& db_conn);

    // PostgreSQL-backed repositories
    static std::unique_ptr<IUserRepository> CreateUserRepository(PostgresConnection& db_conn);
    static std::unique_ptr<IProductRepository> CreateProductRepository(PostgresConnection& db_conn);
    static std::unique_ptr<ICartRepository> CreateCartRepository(PostgresConnection& db_conn);
    static std::unique_ptr<IOrderRepository> CreateOrderRepository(PostgresConnection& db_conn);
    static std::unique_ptr<IReviewRepository> CreateReviewRepository(PostgresConnection& db_conn);
};

} // namespace srikanth::srikanthmart
