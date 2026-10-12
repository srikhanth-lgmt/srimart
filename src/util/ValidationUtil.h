#pragma once

#include <string>
#include <regex>
#include <cctype>
#include "exception/AppException.h"

namespace srikanth::srikanthmart {

class ValidationUtil {
public:
    static void ValidateEmail(const std::string& email) {
        static const std::regex kEmailRegex(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
        if (email.empty() || !std::regex_match(email, kEmailRegex)) {
            throw ValidationException("Invalid email format", "email");
        }
    }

    static void ValidatePassword(const std::string& password) {
        if (password.length() < 8) {
            throw ValidationException("Password must be at least 8 characters long", "password");
        }
        bool lower = false, upper = false, digit = false, symbol = false;
        for (unsigned char c : password) {
            lower = lower || (c >= 'a' && c <= 'z');
            upper = upper || (c >= 'A' && c <= 'Z');
            digit = digit || (c >= '0' && c <= '9');
            symbol = symbol || !(std::isalnum(c));
        }
        const int categories = static_cast<int>(lower) + static_cast<int>(upper) + static_cast<int>(digit) + static_cast<int>(symbol);
        if (categories < 3) throw ValidationException("Password must include at least three of lowercase, uppercase, number, and symbol", "password");
    }

    static void ValidateNonEmpty(const std::string& str, const std::string& field_name, const std::string& message) {
        if (str.find_first_not_of(" \t\n\r") == std::string::npos) {
            throw ValidationException(message, field_name);
        }
    }

    static void ValidatePositiveAmount(int64_t cents, const std::string& field_name) {
        if (cents <= 0) {
            throw ValidationException("Amount must be strictly greater than 0", field_name);
        }
    }

    static void ValidateNonNegativeQuantity(int32_t qty, const std::string& field_name) {
        if (qty < 0) {
            throw ValidationException("Quantity cannot be negative", field_name);
        }
    }

    static void ValidateRating(int32_t rating) {
        if (rating < 1 || rating > 5) {
            throw ValidationException("Rating must be between 1 and 5 stars", "rating");
        }
    }

    static std::string SanitizeHtml(const std::string& input) {
        std::string buffer;
        buffer.reserve(input.size());
        for (char c : input) {
            switch (c) {
                case '&':  buffer.append("&amp;");  break;
                case '\"': buffer.append("&quot;"); break;
                case '\'': buffer.append("&apos;"); break;
                case '<':  buffer.append("&lt;");   break;
                case '>':  buffer.append("&gt;");   break;
                default:   buffer.push_back(c);     break;
            }
        }
        return buffer;
    }
};

} // namespace srikanth::srikanthmart
