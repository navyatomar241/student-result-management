// ============================================================================
// Marks Management Implementation
// Student Result Management System - MITS Gwalior
// ============================================================================

#include "marks.h"
#include "result.h"

nlohmann::json MarksController::handle_get_all(Database& db, const crow::request& req, int& status_code) {
    nlohmann::json res;
    std::string student_id = "";
    if (const char* sid = req.url_params.get("student_id")) {
        student_id = sid;
    }

    std::vector<MarkRecord> list = db.get_marks(student_id);
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& m : list) {
        arr.push_back(m.to_json());
    }

    status_code = 200;
    res["success"] = true;
    res["marks"] = arr;
    res["count"] = static_cast<int>(list.size());
    return res;
}

nlohmann::json MarksController::handle_get_by_id(Database& db, const std::string& marks_id, int& status_code) {
    nlohmann::json res;
    MarkRecord m;
    if (!db.get_mark(marks_id, m)) {
        status_code = 404;
        res["success"] = false;
        res["error"] = "Marks record not found with ID: " + marks_id;
        return res;
    }

    status_code = 200;
    res["success"] = true;
    res["mark"] = m.to_json();
    return res;
}

nlohmann::json MarksController::handle_save(Database& db, const nlohmann::json& body, int& status_code) {
    nlohmann::json res;
    MarkRecord m = MarkRecord::from_json(body);

    std::string err;
    if (!db.add_or_update_mark(m, err)) {
        status_code = 400;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    // Automatically recalculate student's academic result using the dedicated C++ engine!
    ResultRecord calculated_result;
    std::string calc_err;
    bool calc_success = ResultEngine::calculate_student_result(db, m.student_id, calculated_result, calc_err);

    status_code = 201;
    res["success"] = true;
    res["message"] = "Examination marks recorded and result recalculated successfully";
    res["mark"] = m.to_json();
    if (calc_success) {
        res["updated_result"] = calculated_result.to_json();
    }
    return res;
}

nlohmann::json MarksController::handle_update(Database& db, const std::string& marks_id, const nlohmann::json& body, int& status_code) {
    nlohmann::json res;
    MarkRecord existing;
    if (!db.get_mark(marks_id, existing)) {
        status_code = 404;
        res["success"] = false;
        res["error"] = "Marks record not found with ID: " + marks_id;
        return res;
    }

    MarkRecord m = MarkRecord::from_json(body);
    m.marks_id = marks_id;
    if (m.student_id.empty()) m.student_id = existing.student_id;
    if (m.subject_id.empty()) m.subject_id = existing.subject_id;

    std::string err;
    if (!db.add_or_update_mark(m, err)) {
        status_code = 400;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    // Automatically recalculate student result
    ResultRecord calculated_result;
    std::string calc_err;
    bool calc_success = ResultEngine::calculate_student_result(db, m.student_id, calculated_result, calc_err);

    status_code = 200;
    res["success"] = true;
    res["message"] = "Marks updated and result recalculated successfully";
    res["mark"] = m.to_json();
    if (calc_success) {
        res["updated_result"] = calculated_result.to_json();
    }
    return res;
}

nlohmann::json MarksController::handle_delete(Database& db, const std::string& marks_id, int& status_code) {
    nlohmann::json res;
    MarkRecord existing;
    if (!db.get_mark(marks_id, existing)) {
        status_code = 404;
        res["success"] = false;
        res["error"] = "Marks record not found with ID: " + marks_id;
        return res;
    }

    std::string student_id = existing.student_id;
    std::string err;
    if (!db.delete_mark(marks_id, err)) {
        status_code = 400;
        res["success"] = false;
        res["error"] = err;
        return res;
    }

    // Re-evaluate result after deletion
    ResultRecord calculated_result;
    std::string calc_err;
    ResultEngine::calculate_student_result(db, student_id, calculated_result, calc_err);

    status_code = 200;
    res["success"] = true;
    res["message"] = "Marks record removed and result updated";
    return res;
}
