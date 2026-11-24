// ============================================================================
// Dedicated C++ Result Calculation Engine Implementation
// Student Result Management System - MITS Gwalior
// ============================================================================

#include "result.h"
#include <cmath>
#include <sstream>
#include <iomanip>

double ResultEngine::calculate_total_obtained(const std::vector<MarkRecord>& student_marks) {
    double total = 0.0;
    for (const auto& m : student_marks) {
        total += m.marks_obtained;
    }
    return total;
}

double ResultEngine::calculate_maximum_total(const std::vector<MarkRecord>& student_marks) {
    double max_total = 0.0;
    for (const auto& m : student_marks) {
        max_total += m.maximum_marks;
    }
    return max_total;
}

double ResultEngine::calculate_percentage(double total_obtained, double maximum_total) {
    if (maximum_total <= 0.0) {
        return 0.0; // Prevent division by zero
    }
    double raw = (total_obtained / maximum_total) * 100.0;
    // Round to 2 decimal places
    return std::round(raw * 100.0) / 100.0;
}

std::string ResultEngine::assign_grade(double percentage) {
    if (percentage >= 90.0) {
        return "A+";
    } else if (percentage >= 80.0) {
        return "A";
    } else if (percentage >= 70.0) {
        return "B";
    } else if (percentage >= 60.0) {
        return "C";
    } else if (percentage >= 40.0) {
        return "D";
    } else {
        return "Fail";
    }
}

std::string ResultEngine::determine_pass_fail(double percentage) {
    if (percentage >= 40.0) {
        return "Pass";
    }
    return "Fail";
}

bool ResultEngine::calculate_student_result(Database& db, const std::string& student_id, ResultRecord& out_result, std::string& err_msg) {
    StudentRecord student;
    if (!db.get_student(student_id, student)) {
        err_msg = "Student not found with ID: " + student_id;
        return false;
    }

    std::vector<MarkRecord> marks = db.get_marks(student_id);
    if (marks.empty()) {
        err_msg = "Cannot calculate result: No marks have been entered for student " + student_id;
        return false;
    }

    double total_obtained = calculate_total_obtained(marks);
    double max_total = calculate_maximum_total(marks);

    if (max_total <= 0.0) {
        err_msg = "Calculation error: Total maximum marks cannot be zero";
        return false;
    }

    double percentage = calculate_percentage(total_obtained, max_total);
    std::string grade = assign_grade(percentage);
    std::string status = determine_pass_fail(percentage);

    out_result.result_id = "RES_" + student_id;
    out_result.student_id = student_id;
    out_result.student_name = student.name;
    out_result.roll_no = student.roll_no;
    out_result.class_name = student.class_name;
    out_result.total_marks = total_obtained;
    out_result.maximum_total_marks = max_total;
    out_result.percentage = percentage;
    out_result.grade = grade;
    out_result.result_status = status;

    // Persist in database
    db.save_result(out_result);
    return true;
}

int ResultEngine::recalculate_all_results(Database& db, std::string& err_msg) {
    std::vector<StudentRecord> students = db.get_students();
    int count = 0;
    for (const auto& s : students) {
        ResultRecord res;
        std::string local_err;
        if (calculate_student_result(db, s.student_id, res, local_err)) {
            count++;
        }
    }
    return count;
}

nlohmann::json ResultEngine::handle_calculate(Database& db, const nlohmann::json& request_body, int& status_code) {
    nlohmann::json response;
    std::string student_id = request_body.value("student_id", "");

    if (student_id.empty()) {
        // Recalculate for all students
        std::string err;
        int count = recalculate_all_results(db, err);
        status_code = 200;
        response["success"] = true;
        response["message"] = "Successfully calculated results for " + std::to_string(count) + " students";
        response["calculated_count"] = count;
        return response;
    }

    ResultRecord res;
    std::string err_msg;
    if (!calculate_student_result(db, student_id, res, err_msg)) {
        status_code = 400;
        response["success"] = false;
        response["error"] = err_msg;
        return response;
    }

    status_code = 200;
    response["success"] = true;
    response["message"] = "Result calculated successfully";
    response["result"] = res.to_json();
    return response;
}

nlohmann::json ResultEngine::handle_get_all_results(Database& db, int& status_code) {
    nlohmann::json response;
    std::vector<ResultRecord> all_results = db.get_results();
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& r : all_results) {
        arr.push_back(r.to_json());
    }
    status_code = 200;
    response["success"] = true;
    response["results"] = arr;
    return response;
}

nlohmann::json ResultEngine::handle_get_student_result(Database& db, const std::string& student_id, int& status_code) {
    nlohmann::json response;
    StudentRecord student;
    if (!db.get_student(student_id, student)) {
        status_code = 404;
        response["success"] = false;
        response["error"] = "Student not found with ID: " + student_id;
        return response;
    }

    ResultRecord res;
    bool has_result = db.get_result_by_student(student_id, res);

    // If result not found or needs fresh calculation, try calculating on the fly
    if (!has_result) {
        std::string err;
        if (calculate_student_result(db, student_id, res, err)) {
            has_result = true;
        }
    }

    std::vector<MarkRecord> marks = db.get_marks(student_id);
    nlohmann::json marks_arr = nlohmann::json::array();
    for (const auto& m : marks) {
        marks_arr.push_back(m.to_json());
    }

    status_code = 200;
    response["success"] = true;
    response["student"] = student.to_json();
    response["has_result"] = has_result;
    if (has_result) {
        response["result"] = res.to_json();
    }
    response["marks"] = marks_arr;
    return response;
}
