#include <gtest/gtest.h>
#include "service/ProductService.h"
#include "repository/IProductRepository.h"
#include "exception/AppException.h"

using namespace srikanth::srikanthmart;

class FakeProductRepository : public IProductRepository {
public:
    std::optional<Product> FindById(int64_t id) override {
        for (const auto& p : products_) {
            if (p.id == id) return p;
        }
        return std::nullopt;
    }

    std::vector<Product> FindAll(const std::string& query, const std::string& category) override {
        std::vector<Product> result;
        for (const auto& p : products_) {
            bool matches_query = query.empty() || p.name.find(query) != std::string::npos;
            bool matches_cat = category.empty() || p.category == category;
            if (matches_query && matches_cat) {
                result.push_back(p);
            }
        }
        return result;
    }

    std::vector<Product> FindBySellerId(int64_t seller_id) override {
        std::vector<Product> result;
        for (const auto& p : products_) {
            if (p.seller_id == seller_id) result.push_back(p);
        }
        return result;
    }

    Product CreateProduct(const Product& product) override {
        Product p = product;
        p.id = next_id_++;
        products_.push_back(p);
        return p;
    }

    Product UpdateProduct(const Product& product) override {
        for (auto& p : products_) {
            if (p.id == product.id) {
                p = product;
                return p;
            }
        }
        return product;
    }

    bool DeleteProduct(int64_t id) override {
        for (auto it = products_.begin(); it != products_.end(); ++it) {
            if (it->id == id) {
                products_.erase(it);
                return true;
            }
        }
        return false;
    }

private:
    std::vector<Product> products_;
    int64_t next_id_{1};
};

TEST(ProductServiceTest, CreatesProductSuccessfully) {
    auto repo = std::make_shared<FakeProductRepository>();
    ProductService service(repo);

    CreateProductRequestDto req;
    req.name = "Srikanth Special Laptop";
    req.description = "Top of the line laptop";
    req.price_cents = 4500000;
    req.stock_qty = 10;
    req.category = "Electronics";

    auto resp = service.CreateProduct(2, req);
    EXPECT_EQ(resp.name, "Srikanth Special Laptop");
    EXPECT_EQ(resp.price_cents, 4500000);
    EXPECT_EQ(resp.seller_id, 2);
}

TEST(ProductServiceTest, RejectsInvalidPrice) {
    auto repo = std::make_shared<FakeProductRepository>();
    ProductService service(repo);

    CreateProductRequestDto req;
    req.name = "Zero Price Item";
    req.description = "Test";
    req.price_cents = 0;
    req.stock_qty = 5;
    req.category = "Test";

    EXPECT_THROW(service.CreateProduct(2, req), ValidationException);
}
