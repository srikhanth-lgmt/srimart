#include <gtest/gtest.h>
#include "service/AuthService.h"
#include "repository/IUserRepository.h"
#include "exception/AppException.h"

using namespace sudharshan::sudharshanmart;

class FakeUserRepository : public IUserRepository {
public:
    std::optional<User> FindById(int64_t id) override {
        for (const auto& u : users_) {
            if (u.id == id) return u;
        }
        return std::nullopt;
    }

    std::optional<User> FindByEmail(const std::string& email) override {
        for (const auto& u : users_) {
            if (u.email == email) return u;
        }
        return std::nullopt;
    }

    User CreateUser(const User& user) override {
        User u = user;
        u.id = next_id_++;
        users_.push_back(u);
        return u;
    }

    void UpdatePassword(int64_t id, const std::string& hash) override {
        for (auto& user : users_) if (user.id == id) user.password_hash = hash;
    }

    std::vector<User> FindAll() override {
        return users_;
    }

private:
    std::vector<User> users_;
    int64_t next_id_{1};
};

TEST(AuthServiceTest, RegistersNewBuyerSuccessfully) {
    auto fake_repo = std::make_shared<FakeUserRepository>();
    AuthService auth_service(fake_repo);

    RegisterUserRequestDto req;
    req.name = "Sudharshan Buyer";
    req.email = "sudharshan.buyer@test.com";
    req.password = "StrongPass123!";
    req.role = "BUYER";
    req.accepted_terms = true;

    UserResponseDto resp = auth_service.Register(req);
    EXPECT_EQ(resp.name, "Sudharshan Buyer");
    EXPECT_EQ(resp.email, "sudharshan.buyer@test.com");
    EXPECT_EQ(resp.role, "BUYER");
    EXPECT_GT(resp.id, 0);
}

TEST(AuthServiceTest, PreventsDuplicateEmailRegistration) {
    auto fake_repo = std::make_shared<FakeUserRepository>();
    AuthService auth_service(fake_repo);

    RegisterUserRequestDto req;
    req.name = "First User";
    req.email = "duplicate@test.com";
    req.password = "StrongPass123!";
    req.role = "BUYER";
    req.accepted_terms = true;

    auth_service.Register(req);
    EXPECT_THROW(auth_service.Register(req), ConflictException);
}

TEST(AuthServiceTest, RejectsInvalidEmailFormat) {
    auto fake_repo = std::make_shared<FakeUserRepository>();
    AuthService auth_service(fake_repo);

    RegisterUserRequestDto req;
    req.name = "Bad User";
    req.email = "bad-email";
    req.password = "StrongPass123!";
    req.role = "BUYER";
    req.accepted_terms = true;

    EXPECT_THROW(auth_service.Register(req), ValidationException);
}

TEST(AuthServiceTest, RegistersSellerAndNormalizesEmail) {
    auto fake_repo = std::make_shared<FakeUserRepository>();
    AuthService auth_service(fake_repo);
    RegisterUserRequestDto req;
    req.name = "Seller"; req.email = "  Seller@Example.com "; req.password = "StrongPass123!"; req.role = "SELLER"; req.accepted_terms = true;
    const auto response = auth_service.Register(req);
    EXPECT_EQ(response.email, "seller@example.com");
    EXPECT_EQ(response.role, "SELLER");
}

TEST(AuthServiceTest, RejectsAdminPublicRegistration) {
    auto fake_repo = std::make_shared<FakeUserRepository>();
    AuthService auth_service(fake_repo);
    RegisterUserRequestDto req;
    req.name = "Admin"; req.email = "admin@example.com"; req.password = "StrongPass123!"; req.role = "ADMIN"; req.accepted_terms = true;
    EXPECT_THROW(auth_service.Register(req), ValidationException);
}
