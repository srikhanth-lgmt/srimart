#pragma once

#include "repository/IOrderRepository.h"
#include "repository/PostgresConnection.h"

namespace sudharshan::sudharshanmart {

class PostgresOrderRepository : public IOrderRepository {
public:
    explicit PostgresOrderRepository(PostgresConnection& db_conn) : db_conn_(db_conn) {}

    Order CreateOrder(int64_t buyer_id, const std::vector<CartItem>& items, int64_t total_amount_cents) override;
    std::optional<Order> FindById(int64_t order_id) override;
    std::vector<Order> FindByBuyerId(int64_t buyer_id) override;
    std::vector<Order> FindIncomingForSeller(int64_t seller_id) override;
    std::vector<Order> FindAll() override;
    bool UpdateOrderStatus(int64_t order_id, OrderStatus status) override;
    int64_t GetSellerRevenue(int64_t seller_id) override;
    int64_t GetSellerOrderCount(int64_t seller_id) override;

private:
    PostgresConnection& db_conn_;
    void PopulateOrderItems(Order& order);
};

} // namespace sudharshan::sudharshanmart
