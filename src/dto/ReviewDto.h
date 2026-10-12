#pragma once

#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "model/Review.h"

namespace srikanth::srikanthmart {

struct CreateReviewRequestDto {
    int32_t rating{5};
    std::string comment;
};

class ReviewResponseDto {
public:
    int64_t id{0};
    int64_t product_id{0};
    int64_t user_id{0};
    std::string reviewer_name;
    int32_t rating{5};
    std::string comment;
    std::string created_at;

    [[nodiscard]] nlohmann::json ToJson() const {
        return nlohmann::json{
            {"id", id},
            {"product_id", product_id},
            {"user_id", user_id},
            {"reviewer_name", reviewer_name},
            {"rating", rating},
            {"comment", comment},
            {"created_at", created_at}
        };
    }

    // Declared here, defined after the class is complete (see below) so that
    // Builder may hold ReviewResponseDto by value.
    class Builder;

    static ReviewResponseDto FromEntity(const Review& r);
};

class ReviewResponseDto::Builder {
public:
    Builder& SetId(int64_t id) { dto_.id = id; return *this; }
    Builder& SetProductId(int64_t pid) { dto_.product_id = pid; return *this; }
    Builder& SetUserId(int64_t uid) { dto_.user_id = uid; return *this; }
    Builder& SetReviewerName(std::string name) { dto_.reviewer_name = std::move(name); return *this; }
    Builder& SetRating(int32_t rating) { dto_.rating = rating; return *this; }
    Builder& SetComment(std::string comment) { dto_.comment = std::move(comment); return *this; }
    Builder& SetCreatedAt(std::string created_at) { dto_.created_at = std::move(created_at); return *this; }

    [[nodiscard]] ReviewResponseDto Build() const {
        return dto_;
    }
private:
    ReviewResponseDto dto_;
};

inline ReviewResponseDto ReviewResponseDto::FromEntity(const Review& r) {
    return Builder()
        .SetId(r.id)
        .SetProductId(r.product_id)
        .SetUserId(r.user_id)
        .SetReviewerName(r.reviewer_name)
        .SetRating(r.rating)
        .SetComment(r.comment)
        .SetCreatedAt(r.created_at)
        .Build();
}

} // namespace srikanth::srikanthmart
