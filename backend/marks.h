// ============================================================================
// Marks Management Module
// Student Result Management System - MITS Gwalior
// ============================================================================

#ifndef MARKS_H
#define MARKS_H

#include <string>
#include "database.h"
#include "crow.h"
#include "nlohmann/json.hpp"

class MarksController {
public:
    static nlohmann::json handle_get_all(Database& db, const crow::request& req, int& status_code);
    static nlohmann::json handle_get_by_id(Database& db, const std::string& marks_id, int& status_code);
    static nlohmann::json handle_save(Database& db, const nlohmann::json& body, int& status_code);
    static nlohmann::json handle_update(Database& db, const std::string& marks_id, const nlohmann::json& body, int& status_code);
    static nlohmann::json handle_delete(Database& db, const std::string& marks_id, int& status_code);
};

#endif // MARKS_H
