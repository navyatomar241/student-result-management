// ============================================================================
// Teacher Management Implementation
// Student Result Management System - MITS Gwalior
// ============================================================================

#include "teacher.h"

nlohmann::json TeacherController::handle_get_all(Database& db, const crow::request& req, int& status_code) {
    nlohmann::json res;
    std::string query = "";
    if (const char* q = req.url_params.get("q")) {
        query = q;
    }

    std::vector<TeacherRecord> list = db.get_teachers(query);
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& t : list) {
        arr.push_back(t.to_json());
    }

    status_code = 200;
    res["success"] = true;
    res["teachers"] = arr;
    res["count"] = static_cast<int>(list.size());
    return res;
}

nlohmann::json TeacherController::handle_get_by_id(Database& db, const std::string& teacher_id, int& status_code) {
    nlohmann::json res;
    TeacherRecord t;
    if (!db.get_teacher(teacher_id, t)) {
        status_code = 404;
        res["success"] = false;
        res["error"] = "Teacher not found with ID: " + teacher_id;
        return res;
    }

    status_code = 200;
    res["success"] = true;
    res["teacher"] = t.to_json();
    return res;
}

nlohmann::json TeacherController::handle_create(Database& db, const nlohmann::json& body, int& status_code) {
    nlohmann::json res;
    TeacherRecord t = TeacherRecord::from_json(body);

    std::string err;
    if (!db.add_teacher(t, err)) {
        status_code = 400;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    status_code = 201;
    res["success"] = true;
    res["message"] = "Teacher record created successfully";
    res["teacher"] = t.to_json();
    return res;
}

nlohmann::json TeacherController::handle_update(Database& db, const std::string& teacher_id, const nlohmann::json& body, int& status_code) {
    nlohmann::json res;
    TeacherRecord t = TeacherRecord::from_json(body);

    std::string err;
    if (!db.update_teacher(teacher_id, t, err)) {
        status_code = 400;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    TeacherRecord updated;
    db.get_teacher(teacher_id, updated);
    status_code = 200;
    res["success"] = true;
    res["message"] = "Teacher record updated successfully";
    res["teacher"] = updated.to_json();
    return res;
}

nlohmann::json TeacherController::handle_delete(Database& db, const std::string& teacher_id, int& status_code) {
    nlohmann::json res;
    std::string err;
    if (!db.delete_teacher(teacher_id, err)) {
        status_code = 404;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    status_code = 200;
    res["success"] = true;
    res["message"] = "Teacher record removed successfully";
    return res;
}
