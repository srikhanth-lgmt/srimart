#pragma once

#include "repository/ICartRepository.h"
#include "repository/PostgresConnection.h"

namespace sudharshan::sudharshanmart {

class PostgresCartRepository : public ICartRepository {
public:
    explicit PostgresCartRepository(PostgresConnection& db_conn) : db_conn_(db_conn) {}

    std::vector<CartItem> GetCart(int64_t user_id) override;
    CartItem AddOrUpdateItem(int64_t user_id, int64_t product_id, int32_t quantity) override;
    bool RemoveItem(int64_t user_id, int64_t product_id) override;
    void ClearCart(int64_t user_id) override;

private:
    PostgresConnection& db_conn_;
};

} // namespace sudharshan::sudharshanmart
