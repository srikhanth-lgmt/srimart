#include "controller/HealthController.h"
#include "repository/DatabaseConnection.h"
#include "repository/PostgresConnection.h"

namespace sudharshan::sudharshanmart {

void HealthController::CheckHealth(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    std::string request_id = GetRequestId(req);
    std::string db_status = "DOWN";
    nlohmann::json health_data;

    try {
        if (PostgresConnection::GetInstance().IsInitialized()) {
            if (PostgresConnection::GetInstance().Ping()) {
                db_status = "UP";
                const auto info = PostgresConnection::GetInstance().GetDatabaseInfo();
                health_data["database_backend"] = "postgres";
                health_data["database"] = info.database;
                health_data["schema"] = info.schema;
                health_data["public_tables"] = info.public_table_count;
            }
        } else {
            sqlite3* db = DatabaseConnection::GetInstance().GetDb();
            if (db) {
                health_data["database_backend"] = "sqlite";
                sqlite3_stmt* stmt = nullptr;
                if (sqlite3_prepare_v2(db, "SELECT 1;", -1, &stmt, nullptr) == SQLITE_OK) {
                    if (sqlite3_step(stmt) == SQLITE_ROW) {
                        db_status = "UP";
                    }
                    sqlite3_finalize(stmt);
                }
            }
        }
    } catch (...) {
        db_status = "DOWN";
    }

    health_data["status"] = (db_status == "UP") ? "UP" : "DEGRADED";
    health_data["db"] = db_status;
    health_data["app"] = "SudharshanMart";
    health_data["version"] = "1.1.0";

    auto resp = ApiResponse<>::ToHttpResponse(ApiResponse<>::Success(health_data));
    if (db_status != "UP") {
        resp->setStatusCode(drogon::k503ServiceUnavailable);
    }
    callback(resp);
}

} // namespace sudharshan::sudharshanmart
