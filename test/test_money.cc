#include <gtest/gtest.h>
#include "model/Money.h"

using namespace srikanth::srikanthmart;

TEST(MoneyTest, ConstructsFromCents) {
    Money m(4500000);
    EXPECT_EQ(m.GetCents(), 4500000);
    EXPECT_EQ(m.ToFormattedString("INR "), "INR 45000.00");
}

TEST(MoneyTest, ConstructsFromMajorUnits) {
    Money m = Money::FromMajorUnits(180, 50);
    EXPECT_EQ(m.GetCents(), 18050);
    EXPECT_EQ(m.ToFormattedString("INR "), "INR 180.50");
}

TEST(MoneyTest, RejectsInvalidMinorUnits) {
    EXPECT_THROW(Money::FromMajorUnits(10, 150), std::invalid_argument);
    EXPECT_THROW(Money::FromMajorUnits(10, -5), std::invalid_argument);
}

TEST(MoneyTest, PerformsSafeArithmetic) {
    Money m1(1000);
    Money m2(500);

    Money sum = m1 + m2;
    EXPECT_EQ(sum.GetCents(), 1500);

    Money diff = m1 - m2;
    EXPECT_EQ(diff.GetCents(), 500);

    Money prod = m2 * 3;
    EXPECT_EQ(prod.GetCents(), 1500);
}

TEST(MoneyTest, ComparisonsWorkCorrectly) {
    Money low(100);
    Money high(200);

    EXPECT_LT(low, high);
    EXPECT_LE(low, high);
    EXPECT_GT(high, low);
    EXPECT_GE(high, low);
    EXPECT_NE(low, high);
    EXPECT_EQ(low, Money(100));
}
