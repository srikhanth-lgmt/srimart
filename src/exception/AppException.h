#pragma once

#include <stdexcept>
#include <string>
#include <optional>
#include <drogon/drogon.h>

namespace srikanth::srikanthmart {

/**
 * @brief Base application exception with error code, message, and HTTP status.
 */
class AppException : public std::runtime_error {
public:
    AppException(std::string code,
                 std::string message,
                 drogon::HttpStatusCode status = drogon::k400BadRequest,
                 std::optional<std::string> field = std::nullopt)
        : std::runtime_error(message),
          code_(std::move(code)),
          message_(std::move(message)),
          status_(status),
          field_(std::move(field)) {}

    [[nodiscard]] const std::string& GetCode() const noexcept { return code_; }
    [[nodiscard]] const std::string& GetMessage() const noexcept { return message_; }
    [[nodiscard]] drogon::HttpStatusCode GetStatusCode() const noexcept { return status_; }
    [[nodiscard]] const std::optional<std::string>& GetField() const noexcept { return field_; }

private:
    std::string code_;
    std::string message_;
    drogon::HttpStatusCode status_;
    std::optional<std::string> field_;
};

class ValidationException : public AppException {
public:
    ValidationException(std::string message, std::optional<std::string> field = std::nullopt)
        : AppException("VALIDATION_ERROR", std::move(message), drogon::k400BadRequest, std::move(field)) {}
};

class UnauthorizedException : public AppException {
public:
    explicit UnauthorizedException(std::string message = "Authentication required")
        : AppException("UNAUTHORIZED", std::move(message), drogon::k401Unauthorized) {}
};

class ForbiddenException : public AppException {
public:
    explicit ForbiddenException(std::string message = "Access denied for this role or resource")
        : AppException("FORBIDDEN", std::move(message), drogon::k403Forbidden) {}
};

class NotFoundException : public AppException {
public:
    explicit NotFoundException(std::string message = "Resource not found")
        : AppException("NOT_FOUND", std::move(message), drogon::k404NotFound) {}
};

class ConflictException : public AppException {
public:
    explicit ConflictException(std::string message = "Resource conflict detected")
        : AppException("CONFLICT", std::move(message), drogon::k409Conflict) {}
};

class InternalServerException : public AppException {
public:
    explicit InternalServerException(std::string message = "An internal server error occurred")
        : AppException("INTERNAL_SERVER_ERROR", std::move(message), drogon::k500InternalServerError) {}
};

} // namespace srikanth::srikanthmart
