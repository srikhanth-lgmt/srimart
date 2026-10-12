# CAPSTONE PROJECT REPORT

## PROJECT TITLE: SrikanthMart — Native C++20 Multi-Seller Marketplace Backend and Full-Stack Platform
**Student / Project Owner:** Srikanth  
**Course / Track:** C++ Full Stack Capstone Project (Native C++20 Equivalent of Servlets/JDBC Enterprise Systems)  
**Academic Year:** 2025–2026  
**Repository:** `srikanthmart`  
**Namespace:** `srikanth::srikanthmart`  
**Technology Stack:** C++20, Drogon Framework, SQLite3 / PostgreSQL 15+, libsodium (Argon2id), nlohmann-json, spdlog, GoogleTest, HTML5/CSS3/Vanilla JS  

---

## 1. EXECUTIVE SUMMARY

**SrikanthMart** is an enterprise-grade multi-seller e-commerce marketplace platform built entirely with native C++20 using the asynchronous **Drogon** web framework. The project represents a modern, type-safe, and high-performance native equivalent to traditional Java EE (Servlets/JDBC) capstones. 

Sellers can seamlessly register, manage inventory, publish rich product listings, fulfill incoming orders, and track revenue via dedicated analytics dashboards. Buyers enjoy a responsive, dynamic shopping experience featuring instant keyword search, category filtering, cart management with running totals, mock multi-channel checkout, order history with status workflows, and verified customer reviews. In addition, an intelligent domain AI assistant powered by the **Factory and Strategy Patterns** provides real-time customer support with session-level rate limiting and query caching.

The platform was built strictly according to clean architecture, SOLID principles, RAII resource safety, and zero-trust security standards (Argon2id password hashing, 100% parameterized SQL queries, and zero stack trace leakages).

---

## 2. PROBLEM DEFINITION & SCOPE

### 2.1 Problem Statement
Modern web backends frequently suffer from excessive memory overhead and high latency when implemented in interpreted or heavyweight runtimes. The objective was to architect a full-featured, multi-tenant marketplace platform using compiled native C++20 that delivers sub-millisecond response times while preserving strict object-oriented design, testability, and security.

### 2.2 Functional Scope
- **User Authentication & RBAC (F1):** Three distinct roles—Buyer, Seller, and Admin. Admin account is securely seeded during migration for Srikanth.
- **Seller Product Catalog Management (F2):** Create, update, and delete product listings with prices, stock levels, and media URLs.
- **Buyer Exploration & Discovery (F3):** Search and filter by category and keywords without page reloads.
- **Shopping Cart (F4):** Cart state management with automatic quantity validation against live inventory and real-time total calculation.
- **Transactional Checkout (F5):** Atomically convert cart items into persistent orders with mock payment authorization (UPI, Card, NetBanking).
- **Order Tracking Workflow (F6 & O2):** Four-stage state machine (`Pending` → `Confirmed` → `Shipped` → `Delivered`).
- **Administrative Governance (F7):** Central management console for user directories, global order tracking, and product moderation.
- **Verified Product Reviews (F8):** Star ratings and comments permitted strictly on completed purchases.
- **Seller Sales Analytics (O3):** Calculation of aggregate sales volume and revenue.
- **AI Chatbot Assistant (O4):** Floating conversational widget with domain-restricted responses and fallback degradation.

---

## 3. SYSTEM ARCHITECTURE & DESIGN

### 3.1 Tiered Layering Overview
The application follows a strict 5-layer Clean Architecture:
1. **Filter Layer (`src/filter/`):** Drogon HTTP filters enforcing authentication sessions (`AuthFilter`), generating unique request tracking UUIDs (`LoggingFilter`), and managing CORS (`CorsFilter`).
2. **Controller Layer (`src/controller/`):** Thin HTTP routing controllers derived from `BaseController` (Front Controller pattern) that handle HTTP requests, extract parameters, and map exceptions to standard JSON envelopes.
3. **Service Layer (`src/service/`):** Pure C++ business logic classes. Validations execute at the top of each method before repository invocation. Services have zero database dependencies and rely entirely on constructor-injected abstract repository interfaces.
4. **Repository Layer (`src/repository/`):** Abstract repository interfaces (`IUserRepository`, `IProductRepository`, etc.) backed by RAII-governed SQLite/Postgres implementations using 100% parameterized queries.
5. **Data Transfer Object Layer (`src/dto/`):** Clean separation of internal data models from external network payloads. Fluent `Builder` patterns construct outbound DTOs, strictly omitting sensitive data such as password hashes.

### 3.2 Design Patterns Implemented

| Design Pattern | Concrete Class / Location | Role in Architecture |
| :--- | :--- | :--- |
| **Repository** | `src/repository/` | Decouples business rules from underlying SQL syntax |
| **Front Controller** | `BaseController`, Drogon Router | Centralizes authentication extraction and error transformation |
| **Singleton** | `DatabaseConnection`, `DbPlugin` | Governs single database connection pool instance across threads |
| **Factory** | `RepositoryFactory`, `ChatProviderFactory` | Encapsulates concrete class construction (`std::unique_ptr<T>`) |
| **Strategy** | `ChatProvider` (`Gemini` vs `Mock`) | Swappable AI response engines configurable via environment |
| **Builder** | `UserResponseDto::Builder`, `CartSummaryResponseDto::Builder` | Fluent DTO composition |
| **Dependency Injection** | All Services (`AuthService`, `ProductService`, etc.) | Manual constructor injection (`std::shared_ptr<Interface>`) |

---

## 4. DATABASE DESIGN & MIGRATION STRATEGY

### 4.1 Relational Schema
All tables incorporate `created_at` audit timestamps, foreign key constraints, and dedicated indexes:
- `users`: ID (PK), Name, Email (UNIQUE), Password Hash (Argon2id), Role (CHECK: BUYER, SELLER, ADMIN).
- `products`: ID (PK), Seller ID (FK -> users), Name, Description, Price Cents (BIGINT), Stock Qty, Category, Image URL.
- `orders`: ID (PK), Buyer ID (FK -> users), Status (CHECK), Total Amount Cents (BIGINT).
- `order_items`: ID (PK), Order ID (FK -> orders), Product ID (FK -> products), Quantity, Unit Price Cents.
- `cart_items`: ID (PK), User ID (FK -> users), Product ID (FK -> products), Quantity.
- `reviews`: ID (PK), Product ID (FK -> products), User ID (FK -> users), Rating (CHECK 1-5), Comment.

### 4.2 Financial Precision Rule
Floating-point primitives (`float`, `double`) are strictly prohibited for financial fields. Prices are stored as `BIGINT` minor units (cents) and encapsulated within a custom C++ `Money` value type that guarantees overflow protection, safe arithmetic, and standardized currency formatting (`INR 45000.00`).

### 4.3 Database Migrations
Database schema changes are executed by a custom `MigrationService` invoked automatically on startup:
- `V1__init_schema.sql`: Core tables, foreign keys, and indexes.
- `V2__add_reviews_table.sql`: Product reviews and ratings.
- `V3__add_index_orders_status.sql`: Composite indexing for order status workflow optimizations.
- `schema_migrations`: Tracking table guaranteeing idempotent execution.

---

## 5. SECURITY & CODE QUALITY RIGOR

1. **Argon2id Password Hashing:** Powered by libsodium (`crypto_pwhash_str`). No plain passwords or MD5/SHA-1 algorithms are used anywhere.
2. **Parameterized Queries:** Every SQL query utilizes `sqlite3_bind_*` or prepared statements, completely preventing SQL injection.
3. **Session Fixation Defense:** Pre-authentication sessions are cleared upon successful login, issuing fresh session cookies with an explicit 1-hour idle timeout.
4. **Information Disclosure Prevention:** The `GlobalErrorHandler` catches all uncaught exceptions and transforms them into standardized API envelopes (`code`, `message`, `field`), preventing internal stack traces or server paths from leaking.
5. **Memory Safety & Sanitizers:** Code utilizes RAII pointers (`std::unique_ptr` and `std::shared_ptr`) with zero manual `new`/`delete` calls. AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan) compile cleanly in the CI pipeline.

---

## 6. TESTING & VERIFICATION

Automated testing is configured via GoogleTest (`ctest`):
- **Unit Testing:** `test_money.cc` verifies monetary arithmetic and comparisons; `test_validation.cc` verifies regex email validation, password bounds, and HTML sanitization.
- **Service Layer Mocking:** `test_auth_service.cc` and `test_product_service.cc` execute business rules against mock repository fakes (`FakeUserRepository`, `FakeProductRepository`) in total isolation.
- **Chatbot Guardrail Testing:** `test_chat_service.cc` validates 10 req/min rate-limiting throttles and query length constraints.

---

## 7. DEPLOYMENT & DEVOPS

- **Multi-Stage Docker:** Production builds compile in an Ubuntu toolchain stage and copy the standalone binary into a minimal `debian:bookworm-slim` container (approx. 85MB).
- **Systemd Daemon:** Managed via `systemd/srikanthmart.service` with automatic restarts and logging.
- **Reverse Proxy:** Nginx configuration with TLS termination and HTTP/2 proxying to port 8080.
- **CI/CD Pipeline:** GitHub Actions workflow executing `cppcheck`, CMake build with ASan/UBSan, and `ctest` on every push.

---

## 8. CONCLUSION & LEARNING OUTCOMES

The **SrikanthMart** capstone project successfully demonstrates that native C++20 can serve as a robust, maintainable, and remarkably fast foundation for full-stack e-commerce platforms. By adopting modern C++ standards, clean architecture, and rigorous automated testing, the application achieves enterprise reliability and sub-millisecond API response times.
