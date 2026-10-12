#include <gtest/gtest.h>
#include "util/ValidationUtil.h"
#include "exception/AppException.h"

using namespace srikanth::srikanthmart;

TEST(ValidationTest, ValidatesEmailsCorrectly) {
    EXPECT_NO_THROW(ValidationUtil::ValidateEmail("buyer@srikanthmart.com"));
    EXPECT_NO_THROW(ValidationUtil::ValidateEmail("admin@example.org"));
    EXPECT_THROW(ValidationUtil::ValidateEmail("invalid-email"), ValidationException);
    EXPECT_THROW(ValidationUtil::ValidateEmail(""), ValidationException);
}

TEST(ValidationTest, ValidatesPasswordLength) {
    EXPECT_NO_THROW(ValidationUtil::ValidatePassword("Password123!"));
    EXPECT_THROW(ValidationUtil::ValidatePassword("short"), ValidationException);
}

TEST(ValidationTest, ValidatesPositiveAmount) {
    EXPECT_NO_THROW(ValidationUtil::ValidatePositiveAmount(100, "price"));
    EXPECT_THROW(ValidationUtil::ValidatePositiveAmount(0, "price"), ValidationException);
    EXPECT_THROW(ValidationUtil::ValidatePositiveAmount(-50, "price"), ValidationException);
}

TEST(ValidationTest, ValidatesRatings) {
    EXPECT_NO_THROW(ValidationUtil::ValidateRating(1));
    EXPECT_NO_THROW(ValidationUtil::ValidateRating(5));
    EXPECT_THROW(ValidationUtil::ValidateRating(0), ValidationException);
    EXPECT_THROW(ValidationUtil::ValidateRating(6), ValidationException);
}

TEST(ValidationTest, SanitizesHtmlSpecialChars) {
    std::string xss = "<script>alert('xss');</script>";
    std::string safe = ValidationUtil::SanitizeHtml(xss);
    EXPECT_EQ(safe, "&lt;script&gt;alert(&apos;xss&apos;);&lt;/script&gt;");
}
