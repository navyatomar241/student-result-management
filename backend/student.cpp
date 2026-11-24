// ============================================================================
// Student Management Implementation
// Student Result Management System - MITS Gwalior
// ============================================================================

#include "student.h"

nlohmann::json StudentController::handle_get_all(Database& db, const crow::request& req, int& status_code) {
    nlohmann::json res;
    std::string query = "";
    if (const char* q = req.url_params.get("q")) {
        query = q;
    }

    std::vector<StudentRecord> list = db.get_students(query);
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& s : list) {
        arr.push_back(s.to_json());
    }

    status_code = 200;
    res["success"] = true;
    res["students"] = arr;
    res["count"] = static_cast<int>(list.size());
    return res;
}

nlohmann::json StudentController::handle_get_by_id(Database& db, const std::string& student_id, int& status_code) {
    nlohmann::json res;
    StudentRecord s;
    if (!db.get_student(student_id, s)) {
        status_code = 404;
        res["success"] = false;
        res["error"] = "Student not found with ID: " + student_id;
        return res;
    }

    status_code = 200;
    res["success"] = true;
    res["student"] = s.to_json();
    return res;
}

nlohmann::json StudentController::handle_create(Database& db, const nlohmann::json& body, int& status_code) {
    nlohmann::json res;
    StudentRecord s = StudentRecord::from_json(body);

    std::string err;
    if (!db.add_student(s, err)) {
        status_code = 400;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    status_code = 201;
    res["success"] = true;
    res["message"] = "Student added successfully";
    res["student"] = s.to_json();
    return res;
}

nlohmann::json StudentController::handle_update(Database& db, const std::string& student_id, const nlohmann::json& body, int& status_code) {
    nlohmann::json res;
    StudentRecord s = StudentRecord::from_json(body);

    std::string err;
    if (!db.update_student(student_id, s, err)) {
        status_code = 400;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    StudentRecord updated;
    db.get_student(student_id, updated);
    status_code = 200;
    res["success"] = true;
    res["message"] = "Student record updated successfully";
    res["student"] = updated.to_json();
    return res;
}

nlohmann::json StudentController::handle_delete(Database& db, const std::string& student_id, int& status_code) {
    nlohmann::json res;
    std::string err;
    if (!db.delete_student(student_id, err)) {
        status_code = 404;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    status_code = 200;
    res["success"] = true;
    res["message"] = "Student record and associated marks/results deleted successfully";
    return res;
}
