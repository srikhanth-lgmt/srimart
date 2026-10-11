#pragma once

#include "repository/IReviewRepository.h"
#include "repository/PostgresConnection.h"

namespace sudharshan::sudharshanmart {

class PostgresReviewRepository : public IReviewRepository {
public:
    explicit PostgresReviewRepository(PostgresConnection& db_conn) : db_conn_(db_conn) {}

    Review CreateReview(const Review& review) override;
    std::vector<Review> FindByProductId(int64_t product_id) override;
    bool HasUserPurchasedProduct(int64_t user_id, int64_t product_id) override;

private:
    PostgresConnection& db_conn_;
};

} // namespace sudharshan::sudharshanmart
