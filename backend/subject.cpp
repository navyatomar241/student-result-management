// ============================================================================
// Subject Management Implementation
// Student Result Management System - MITS Gwalior
// ============================================================================

#include "subject.h"

nlohmann::json SubjectController::handle_get_all(Database& db, int& status_code) {
    nlohmann::json res;
    std::vector<SubjectRecord> list = db.get_subjects();
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& s : list) {
        arr.push_back(s.to_json());
    }

    status_code = 200;
    res["success"] = true;
    res["subjects"] = arr;
    res["count"] = static_cast<int>(list.size());
    return res;
}

nlohmann::json SubjectController::handle_get_by_id(Database& db, const std::string& subject_id, int& status_code) {
    nlohmann::json res;
    SubjectRecord s;
    if (!db.get_subject(subject_id, s)) {
        status_code = 404;
        res["success"] = false;
        res["error"] = "Subject not found with ID: " + subject_id;
        return res;
    }

    status_code = 200;
    res["success"] = true;
    res["subject"] = s.to_json();
    return res;
}

nlohmann::json SubjectController::handle_create(Database& db, const nlohmann::json& body, int& status_code) {
    nlohmann::json res;
    SubjectRecord s = SubjectRecord::from_json(body);

    std::string err;
    if (!db.add_subject(s, err)) {
        status_code = 400;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    status_code = 201;
    res["success"] = true;
    res["message"] = "Subject added successfully";
    res["subject"] = s.to_json();
    return res;
}

nlohmann::json SubjectController::handle_update(Database& db, const std::string& subject_id, const nlohmann::json& body, int& status_code) {
    nlohmann::json res;
    SubjectRecord s = SubjectRecord::from_json(body);

    std::string err;
    if (!db.update_subject(subject_id, s, err)) {
        status_code = 400;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    SubjectRecord updated;
    db.get_subject(subject_id, updated);
    status_code = 200;
    res["success"] = true;
    res["message"] = "Subject updated successfully";
    res["subject"] = updated.to_json();
    return res;
}

nlohmann::json SubjectController::handle_delete(Database& db, const std::string& subject_id, int& status_code) {
    nlohmann::json res;
    std::string err;
    if (!db.delete_subject(subject_id, err)) {
        status_code = 400;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    status_code = 200;
    res["success"] = true;
    res["message"] = "Subject deleted successfully";
    return res;
}
