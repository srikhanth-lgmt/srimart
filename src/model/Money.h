#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>

namespace srikanth::srikanthmart {

/**
 * @brief Represents a monetary value safely encapsulated in minor units (cents).
 * Prohibits floating-point currency representation and arithmetic leaks.
 */
class Money {
public:
    constexpr Money() noexcept : cents_(0) {}
    constexpr explicit Money(int64_t cents) noexcept : cents_(cents) {}

    [[nodiscard]] constexpr int64_t GetCents() const noexcept {
        return cents_;
    }

    [[nodiscard]] std::string ToFormattedString(const std::string& currency_symbol = "INR ") const {
        int64_t major = cents_ / 100;
        int64_t minor = cents_ % 100;
        if (minor < 0) {
            minor = -minor;
        }
        std::ostringstream oss;
        oss << currency_symbol << major << "." << std::setfill('0') << std::setw(2) << minor;
        return oss.str();
    }

    [[nodiscard]] static Money FromMajorUnits(int64_t major, int64_t minor = 0) {
        if (minor < 0 || minor >= 100) {
            throw std::invalid_argument("Minor units must be between 0 and 99");
        }
        return Money((major * 100) + minor);
    }

    constexpr Money operator+(const Money& other) const noexcept {
        return Money(cents_ + other.cents_);
    }

    constexpr Money operator-(const Money& other) const noexcept {
        return Money(cents_ - other.cents_);
    }

    constexpr Money operator*(int64_t multiplier) const noexcept {
        return Money(cents_ * multiplier);
    }

    constexpr bool operator==(const Money& other) const noexcept {
        return cents_ == other.cents_;
    }

    constexpr bool operator!=(const Money& other) const noexcept {
        return cents_ != other.cents_;
    }

    constexpr bool operator<(const Money& other) const noexcept {
        return cents_ < other.cents_;
    }

    constexpr bool operator<=(const Money& other) const noexcept {
        return cents_ <= other.cents_;
    }

    constexpr bool operator>(const Money& other) const noexcept {
        return cents_ > other.cents_;
    }

    constexpr bool operator>=(const Money& other) const noexcept {
        return cents_ >= other.cents_;
    }

private:
    int64_t cents_{0};
};

} // namespace srikanth::srikanthmart
