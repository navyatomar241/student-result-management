// ============================================================================
// Dedicated C++ Result Calculation Engine
// Student Result Management System - MITS Gwalior
// ============================================================================

#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <vector>
#include "database.h"
#include "nlohmann/json.hpp"

class ResultEngine {
public:
    // Core C++ calculation functions according to project specifications
    static double calculate_total_obtained(const std::vector<MarkRecord>& student_marks);
    static double calculate_maximum_total(const std::vector<MarkRecord>& student_marks);
    static double calculate_percentage(double total_obtained, double maximum_total);
    static std::string assign_grade(double percentage);
    static std::string determine_pass_fail(double percentage);

    // High-level Result Calculation Methods
    static bool calculate_student_result(Database& db, const std::string& student_id, ResultRecord& out_result, std::string& err_msg);
    static int recalculate_all_results(Database& db, std::string& err_msg);

    // Controller handlers for Crow API
    static nlohmann::json handle_calculate(Database& db, const nlohmann::json& request_body, int& status_code);
    static nlohmann::json handle_get_all_results(Database& db, int& status_code);
    static nlohmann::json handle_get_student_result(Database& db, const std::string& student_id, int& status_code);
};

#endif // RESULT_H
