// ============================================================================
// Subject Management Module
// Student Result Management System - MITS Gwalior
// ============================================================================

#ifndef SUBJECT_H
#define SUBJECT_H

#include <string>
#include "database.h"
#include "crow.h"
#include "nlohmann/json.hpp"

class SubjectController {
public:
    static nlohmann::json handle_get_all(Database& db, int& status_code);
    static nlohmann::json handle_get_by_id(Database& db, const std::string& subject_id, int& status_code);
    static nlohmann::json handle_create(Database& db, const nlohmann::json& body, int& status_code);
    static nlohmann::json handle_update(Database& db, const std::string& subject_id, const nlohmann::json& body, int& status_code);
    static nlohmann::json handle_delete(Database& db, const std::string& subject_id, int& status_code);
};

#endif // SUBJECT_H
