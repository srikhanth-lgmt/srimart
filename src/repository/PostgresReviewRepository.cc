#include "repository/PostgresReviewRepository.h"
#include "exception/AppException.h"
#include <spdlog/spdlog.h>

namespace sudharshan::sudharshanmart {

Review PostgresReviewRepository::CreateReview(const Review& review) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "INSERT INTO reviews (product_id, user_id, rating, comment) VALUES ($1, $2, $3, $4) RETURNING id;",
        { std::to_string(review.product_id), std::to_string(review.user_id),
          std::to_string(review.rating), review.comment });
    if (!res.IsOk()) {
        throw InternalServerException(std::string("Failed to insert review: ") + PQerrorMessage(conn));
    }

    Review created = review;
    created.id = PgInt64(res.Get(), 0, 0);
    return created;
}

std::vector<Review> PostgresReviewRepository::FindByProductId(int64_t product_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "SELECT r.id, r.product_id, r.user_id, r.rating, r.comment, "
        "to_char(r.created_at, 'YYYY-MM-DD HH24:MI:SS') AS created_at, u.name "
        "FROM reviews r "
        "JOIN users u ON r.user_id = u.id "
        "WHERE r.product_id = $1 "
        "ORDER BY r.id DESC;",
        { std::to_string(product_id) });
    if (!res.IsOk()) {
        throw InternalServerException("Failed to prepare reviews query statement");
    }

    std::vector<Review> list;
    const int rows = PQntuples(res.Get());
    for (int i = 0; i < rows; ++i) {
        Review r;
        r.id = PgInt64(res.Get(), i, 0);
        r.product_id = PgInt64(res.Get(), i, 1);
        r.user_id = PgInt64(res.Get(), i, 2);
        r.rating = PgInt32(res.Get(), i, 3);
        r.comment = PgText(res.Get(), i, 4);
        r.created_at = PgText(res.Get(), i, 5);
        r.reviewer_name = PgText(res.Get(), i, 6);
        list.push_back(r);
    }
    return list;
}

bool PostgresReviewRepository::HasUserPurchasedProduct(int64_t user_id, int64_t product_id) {
    std::lock_guard<std::mutex> lock(db_conn_.GetMutex());
    PGconn* conn = db_conn_.Get();

    PgResult res = PgExecParams(conn,
        "SELECT 1 FROM orders o "
        "JOIN order_items oi ON o.id = oi.order_id "
        "WHERE o.buyer_id = $1 AND oi.product_id = $2 "
        "LIMIT 1;",
        { std::to_string(user_id), std::to_string(product_id) });
    if (!res.IsOk()) {
        return false;
    }
    return PQntuples(res.Get()) > 0;
}

} // namespace sudharshan::sudharshanmart
