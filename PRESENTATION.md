# SrikanthMart — Capstone Defense Presentation Deck

**Project Name:** SrikanthMart — Native C++20 Multi-Seller Marketplace  
**Student & Presenter:** Srikanth  
**Evaluator Window:** Jul 27 – Oct 10, 2026  
**Platform Architecture:** C++20 · Drogon · SQLite/PostgreSQL · Argon2id · Clean Architecture  

---

## Slide 1: Title & Introduction
- **Slide Title:** SrikanthMart: High-Performance Native C++20 Marketplace
- **Subtitle:** The Native C++20 Equivalent of the Servlets/JDBC Enterprise Capstone
- **Presenter:** Srikanth (Solo Builder & Lead Architect)
- **Key Visual:** System architecture badge, Drogon + C++20 logo, Live demo link
- **Speaker Notes:**
  > "Good morning/afternoon, evaluators. My name is Srikanth, and I am proud to present SrikanthMart—a native C++20 multi-seller e-commerce web platform engineered with Drogon. This project bridges high-performance systems programming with enterprise web architecture."

---

## Slide 2: Problem Statement & Objectives
- **Slide Title:** Why Native C++ for E-Commerce?
- **Key Points:**
  - Modern web services in interpreted runtimes often consume excessive memory and encounter garbage collection latency spikes.
  - Native C++20 offers sub-millisecond query execution, minimal memory footprints, and compile-time type safety.
  - Objective: Deliver a multi-seller platform with role-based workflows (Buyer, Seller, Admin), ACID transactions, and AI customer assistance.
- **Speaker Notes:**
  > "Our goal was to prove that a modern C++20 backend can deliver the same rich developer ergonomics and security as enterprise frameworks like Spring or Servlets, while operating at a fraction of the hardware footprint."

---

## Slide 3: Clean Multi-Tier Architecture
- **Slide Title:** Architectural Layering (Separation of Concerns)
- **Diagram Highlights:**
  - **Filter Layer:** Session inspection (`AuthFilter`), UUID tagging (`LoggingFilter`), CORS headers (`CorsFilter`).
  - **Controller Layer:** Thin Drogon HttpControllers dispatching requests and handling JSON envelopes.
  - **Service Layer:** Pure C++ business rules, zero database knowledge, isolated and testable.
  - **Repository Layer:** Abstract interfaces (`I...Repository`) backed by parameterized SQL and RAII statements.
  - **Database Connection Pool:** Singleton plugin managing SQLite3 / PostgreSQL instances.
- **Speaker Notes:**
  > "We adopted a strict 5-tier architecture. Notice that the Service layer has no database calls; it relies entirely on abstract interfaces injected through constructors. This strictly honors the Dependency Inversion Principle."

---

## Slide 4: Key Design Patterns Applied
- **Slide Title:** Enterprise Design Patterns in C++20
- **Summary Table:**
  - **Repository Pattern:** Abstract data access (`IProductRepository`, `IOrderRepository`).
  - **Front Controller:** Common base controller (`BaseController`) standardizing session handling.
  - **Singleton Pattern:** Thread-safe database connection pool via `DbPlugin`.
  - **Factory Pattern:** Polymorphic construction of repositories and AI providers returning `unique_ptr`.
  - **Strategy Pattern:** Swappable AI providers (`GeminiChatProvider` vs `MockChatProvider`).
  - **Builder Pattern:** Fluent construction of outbound DTOs (`UserResponseDto::Builder`).
- **Speaker Notes:**
  > "Every core design pattern prescribed by the capstone specification is implemented cleanly, ensuring high modularity and maintainability."

---

## Slide 5: Core Functional Deliverables (F1 – F8)
- **Slide Title:** End-to-End Functional Capabilities
- **Feature Matrix:**
  - **F1 (Authentication):** Role-based access for Buyer, Seller, and seeded Admin (Srikanth).
  - **F2 (Seller Desk):** Product creation, price management, stock tracking, and listing updates.
  - **F3 (Buyer Discovery):** Real-time keyword search and category filtering.
  - **F4 (Shopping Cart):** Live quantity recalculation, stock checks, and running total display.
  - **F5 (Mock Checkout):** Multi-rail mock payment authorization (UPI, Cards, NetBanking).
  - **F6 (Order History):** Buyer order tracking and seller incoming order dispatch views.
  - **F7 (Admin Portal):** System directory oversight and listing moderation.
  - **F8 (Reviews):** Verified customer feedback on completed purchases.
- **Speaker Notes:**
  > "All eight mandatory functional requirements—plus the bonus workflows—are fully functional, integrated, and verified against our database."

---

## Slide 6: Security & Data Integrity Rigor
- **Slide Title:** Zero-Trust Security Engineering
- **Highlights:**
  - **Argon2id Hashing:** Industry-standard password hashing via libsodium (`crypto_pwhash_str`). Plaintext storage is impossible.
  - **100% Parameterized SQL:** All SQL operations use prepared statements. Zero raw string concatenation.
  - **Financial Safety (Money Class):** Floating-point types (`float`, `double`) are strictly prohibited. Amounts are stored as `BIGINT` minor units (cents) wrapped in a custom `Money` type.
  - **Zero Leakage:** Passwords hashes are filtered out of all DTOs; global error handlers prevent internal path disclosure.
- **Speaker Notes:**
  > "Security was treated as a first-class requirement. From Argon2id to RAII transaction guards and our custom Money value type, financial and user data is protected against tampering and rounding errors."

---

## Slide 7: Real-Time AI Chatbot Integration
- **Slide Title:** AI Customer Service Assistant
- **Architecture Highlights:**
  - Floating responsive chat widget communicating with `/api/v1/chat`.
  - Server-side Gemini LLM integration wrapped in timeouts and fallback fallbacks.
  - Standalone `MockChatProvider` for offline or local evaluation.
  - Thread-safe session rate limiting (10 queries/minute) and in-memory question caching.
- **Speaker Notes:**
  > "Our AI Chatbot utilizes the Strategy Pattern. Evaluators can run it with live Gemini API keys or with the bundled Mock provider, which answers shipping, return, and catalog questions with zero latency."

---

## Slide 8: Live Demonstration & Workflow
- **Slide Title:** Live System Demonstration
- **Demo Walkthrough Steps:**
  1. Explore marketplace catalog, search for 'Laptop', and view detail modal.
  2. Add items to cart, view running total, and complete mock checkout.
  3. View order history and observe the live status workflow (`Pending` → `Delivered`).
  4. Switch to Seller Desk to inspect incoming orders and sales revenue dashboard.
  5. Open the AI Assistant to query order status workflow and store policies.
- **Speaker Notes:**
  > "Let us now transition to our live demo running on localhost port 8080."

---

## Slide 9: DevOps, CI/CD & Testing
- **Slide Title:** Automated Verification & Containerization
- **DevOps Matrix:**
  - **Testing:** GoogleTest suites covering value types, service logic mocks, and rate limiting.
  - **Static Analysis:** Configured with `cppcheck` and `clang-tidy`.
  - **CI Pipeline:** GitHub Actions workflow executing CMake build and `ctest` on every push.
  - **Docker:** Multi-stage build producing an 85MB Debian runtime container.
- **Speaker Notes:**
  > "Our project includes a full GitHub Actions workflow, multi-stage Docker deployment, and GoogleTest suites ensuring zero regressions."

---

## Slide 10: Conclusion & Q&A
- **Slide Title:** Conclusion & Evaluation
- **Summary:**
  - High-performance native C++20 marketplace completed by Srikanth.
  - 100% compliance with all capstone requirements, design patterns, and security constraints.
  - Fully packaged with C++ source, SQLite migrations, interactive runner, and complete documentation.
- **Thank you! Questions & Answers.**
- **Speaker Notes:**
  > "Thank you for your time. I am now open to your questions and code inspection."
