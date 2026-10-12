#include "repository/SqliteReviewRepository.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace srikanth::srikanthmart {

Review SqliteReviewRepository::CreateReview(const Review& review) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "INSERT INTO reviews (product_id, user_id, rating, comment) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException("Failed to prepare create review statement");
    }
    StatementRAII stmt(stmt_raw);

    sqlite3_bind_int64(stmt.Get(), 1, review.product_id);
    sqlite3_bind_int64(stmt.Get(), 2, review.user_id);
    sqlite3_bind_int(stmt.Get(), 3, review.rating);
    sqlite3_bind_text(stmt.Get(), 4, review.comment.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt.Get()) != SQLITE_DONE) {
        throw InternalServerException(std::string("Failed to insert review: ") + sqlite3_errmsg(db));
    }

    Review created = review;
    created.id = sqlite3_last_insert_rowid(db);
    return created;
}

std::vector<Review> SqliteReviewRepository::FindByProductId(int64_t product_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT r.id, r.product_id, r.user_id, r.rating, r.comment, r.created_at, u.name "
                      "FROM reviews r "
                      "JOIN users u ON r.user_id = u.id "
                      "WHERE r.product_id = ? "
                      "ORDER BY r.id DESC;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw InternalServerException("Failed to prepare reviews query statement");
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, product_id);

    std::vector<Review> list;
    while (sqlite3_step(stmt.Get()) == SQLITE_ROW) {
        Review r;
        r.id = sqlite3_column_int64(stmt.Get(), 0);
        r.product_id = sqlite3_column_int64(stmt.Get(), 1);
        r.user_id = sqlite3_column_int64(stmt.Get(), 2);
        r.rating = sqlite3_column_int(stmt.Get(), 3);
        r.comment = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 4));
        r.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 5));
        const char* reviewer = reinterpret_cast<const char*>(sqlite3_column_text(stmt.Get(), 6));
        r.reviewer_name = reviewer ? reviewer : "Anonymous";
        list.push_back(r);
    }
    return list;
}

bool SqliteReviewRepository::HasUserPurchasedProduct(int64_t user_id, int64_t product_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    sqlite3* db = db_conn_.GetDb();

    const char* sql = "SELECT 1 FROM orders o "
                      "JOIN order_items oi ON o.id = oi.order_id "
                      "WHERE o.buyer_id = ? AND oi.product_id = ? "
                      "LIMIT 1;";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        return false;
    }
    StatementRAII stmt(stmt_raw);
    sqlite3_bind_int64(stmt.Get(), 1, user_id);
    sqlite3_bind_int64(stmt.Get(), 2, product_id);

    return sqlite3_step(stmt.Get()) == SQLITE_ROW;
}

} // namespace srikanth::srikanthmart
