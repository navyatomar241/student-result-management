// ============================================================================
// Comprehensive Unit & Edge Case Test Suite
// Student Result Management System - MITS Gwalior
// Tests all 20 requirements and edge cases from Section 15
// ============================================================================

#include <iostream>
#include <cassert>
#include <cmath>
#include "database.h"
#include "auth.h"
#include "result.h"
#include "student.h"
#include "teacher.h"
#include "subject.h"
#include "marks.h"

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[-] FAILED: " << msg << "\n"; \
            return false; \
        } else { \
            std::cout << "[+] PASSED: " << msg << "\n"; \
        } \
    } while (0)

bool test_calculation_edge_cases() {
    std::cout << "\n--- Testing Academic Calculation & Grading Edge Cases ---\n";

    // 1. Zero marks
    double p_zero = ResultEngine::calculate_percentage(0.0, 500.0);
    TEST_ASSERT(p_zero == 0.0, "Zero marks percentage is 0.00%");
    TEST_ASSERT(ResultEngine::assign_grade(p_zero) == "Fail", "Zero marks grade is 'Fail'");
    TEST_ASSERT(ResultEngine::determine_pass_fail(p_zero) == "Fail", "Zero marks status is 'Fail'");

    // 2. Below 40% (e.g. 39.80%)
    double p_below40 = ResultEngine::calculate_percentage(199.0, 500.0);
    TEST_ASSERT(p_below40 == 39.80, "199/500 percentage is 39.80%");
    TEST_ASSERT(ResultEngine::assign_grade(p_below40) == "Fail", "39.80% grade is 'Fail'");
    TEST_ASSERT(ResultEngine::determine_pass_fail(p_below40) == "Fail", "39.80% status is 'Fail'");

    // 3. Exactly 40.00%
    double p_exact40 = ResultEngine::calculate_percentage(200.0, 500.0);
    TEST_ASSERT(p_exact40 == 40.00, "200/500 percentage is 40.00%");
    TEST_ASSERT(ResultEngine::assign_grade(p_exact40) == "D", "Exactly 40.00% grade is 'D'");
    TEST_ASSERT(ResultEngine::determine_pass_fail(p_exact40) == "Pass", "Exactly 40.00% status is 'Pass'");

    // 4. Exactly 60.00%
    double p_exact60 = ResultEngine::calculate_percentage(300.0, 500.0);
    TEST_ASSERT(p_exact60 == 60.00, "300/500 percentage is 60.00%");
    TEST_ASSERT(ResultEngine::assign_grade(p_exact60) == "C", "Exactly 60.00% grade is 'C'");
    TEST_ASSERT(ResultEngine::determine_pass_fail(p_exact60) == "Pass", "Exactly 60.00% status is 'Pass'");

    // 5. Exactly 70.00%
    double p_exact70 = ResultEngine::calculate_percentage(350.0, 500.0);
    TEST_ASSERT(p_exact70 == 70.00, "350/500 percentage is 70.00%");
    TEST_ASSERT(ResultEngine::assign_grade(p_exact70) == "B", "Exactly 70.00% grade is 'B'");
    TEST_ASSERT(ResultEngine::determine_pass_fail(p_exact70) == "Pass", "Exactly 70.00% status is 'Pass'");

    // 6. Exactly 80.00%
    double p_exact80 = ResultEngine::calculate_percentage(400.0, 500.0);
    TEST_ASSERT(p_exact80 == 80.00, "400/500 percentage is 80.00%");
    TEST_ASSERT(ResultEngine::assign_grade(p_exact80) == "A", "Exactly 80.00% grade is 'A'");
    TEST_ASSERT(ResultEngine::determine_pass_fail(p_exact80) == "Pass", "Exactly 80.00% status is 'Pass'");

    // 7. Exactly 90.00%
    double p_exact90 = ResultEngine::calculate_percentage(450.0, 500.0);
    TEST_ASSERT(p_exact90 == 90.00, "450/500 percentage is 90.00%");
    TEST_ASSERT(ResultEngine::assign_grade(p_exact90) == "A+", "Exactly 90.00% grade is 'A+'");
    TEST_ASSERT(ResultEngine::determine_pass_fail(p_exact90) == "Pass", "Exactly 90.00% status is 'Pass'");

    // 8. Full marks (100.00%)
    double p_full = ResultEngine::calculate_percentage(500.0, 500.0);
    TEST_ASSERT(p_full == 100.00, "500/500 percentage is 100.00%");
    TEST_ASSERT(ResultEngine::assign_grade(p_full) == "A+", "100.00% grade is 'A+'");
    TEST_ASSERT(ResultEngine::determine_pass_fail(p_full) == "Pass", "100.00% status is 'Pass'");

    // 9. Division by zero protection
    double p_zero_div = ResultEngine::calculate_percentage(100.0, 0.0);
    TEST_ASSERT(p_zero_div == 0.0, "Division by zero is safely handled without crash");

    return true;
}

bool test_database_crud() {
    std::cout << "\n--- Testing Database CRUD & Constraints ---\n";
    Database db;
    db.init("", "data/test_db.json");

    // 1. Initial seeded students
    auto students = db.get_students();
    TEST_ASSERT(students.size() >= 5, "Database initialized with seeded students");

    // 2. Add new student
    StudentRecord new_s;
    new_s.student_id = "STU999";
    new_s.name = "Test Student";
    new_s.class_name = "B.Tech CSE III Sem";
    new_s.roll_no = "0901CS241999";
    std::string err;
    TEST_ASSERT(db.add_student(new_s, err), "New student added successfully");

    // 3. Prevent duplicate student_id
    TEST_ASSERT(!db.add_student(new_s, err), "Duplicate student ID rejected");

    // 4. Prevent duplicate roll_no
    StudentRecord dup_roll = new_s;
    dup_roll.student_id = "STU998";
    TEST_ASSERT(!db.add_student(dup_roll, err), "Duplicate roll number rejected");

    // 5. Search student
    auto found = db.get_students("0901CS241999");
    TEST_ASSERT(found.size() == 1 && found[0].name == "Test Student", "Search by roll number works");

    // 6. Update student
    new_s.name = "Test Student Updated";
    TEST_ASSERT(db.update_student("STU999", new_s, err), "Student updated successfully");
    StudentRecord verify_update;
    db.get_student("STU999", verify_update);
    TEST_ASSERT(verify_update.name == "Test Student Updated", "Updated student name persisted");

    // 7. Delete student
    TEST_ASSERT(db.delete_student("STU999", err), "Student deleted successfully");
    TEST_ASSERT(!db.get_student("STU999", verify_update), "Student ID no longer exists");

    // 8. Subject Management & Dependency constraint
    SubjectRecord new_sub;
    new_sub.subject_id = "SUB999";
    new_sub.subject_name = "Cloud Computing";
    TEST_ASSERT(db.add_subject(new_sub, err), "Subject added successfully");
    // Can delete unreferenced subject
    TEST_ASSERT(db.delete_subject("SUB999", err), "Subject deleted when unreferenced");

    // Cannot delete subject referenced by marks (SUB101)
    TEST_ASSERT(!db.delete_subject("SUB101", err), "Subject with existing marks cannot be deleted");

    // 9. Marks Management & Validation
    MarkRecord invalid_neg;
    invalid_neg.student_id = "STU001";
    invalid_neg.subject_id = "SUB101";
    invalid_neg.marks_obtained = -10.0;
    invalid_neg.maximum_marks = 100.0;
    TEST_ASSERT(!db.add_or_update_mark(invalid_neg, err), "Negative marks rejected");

    MarkRecord invalid_exceed;
    invalid_exceed.student_id = "STU001";
    invalid_exceed.subject_id = "SUB101";
    invalid_exceed.marks_obtained = 105.0;
    invalid_exceed.maximum_marks = 100.0;
    TEST_ASSERT(!db.add_or_update_mark(invalid_exceed, err), "Obtained marks exceeding maximum rejected");

    // Valid mark update
    MarkRecord valid_m;
    valid_m.student_id = "STU001";
    valid_m.subject_id = "SUB101";
    valid_m.marks_obtained = 98.0;
    valid_m.maximum_marks = 100.0;
    TEST_ASSERT(db.add_or_update_mark(valid_m, err), "Valid marks update accepted");

    // Result recalculation
    ResultRecord res;
    TEST_ASSERT(ResultEngine::calculate_student_result(db, "STU001", res, err), "Result recalculated after marks change");
    TEST_ASSERT(res.grade == "A+" && res.result_status == "Pass", "STU001 result calculated correctly as A+ Pass");

    return true;
}

bool test_auth_and_roles() {
    std::cout << "\n--- Testing Authentication & Role Permissions ---\n";
    Database db;
    db.init("", "data/test_db.json");

    std::string err;
    AuthContext admin_ctx = AuthManager::verify_token("Bearer demo-admin-token", db, err);
    TEST_ASSERT(admin_ctx.is_authenticated && admin_ctx.role == "admin", "Admin role verified");

    AuthContext teacher_ctx = AuthManager::verify_token("Bearer demo-teacher-token", db, err);
    TEST_ASSERT(teacher_ctx.is_authenticated && teacher_ctx.role == "teacher", "Teacher role verified");

    AuthContext student_ctx = AuthManager::verify_token("Bearer demo-student-token", db, err);
    TEST_ASSERT(student_ctx.is_authenticated && student_ctx.role == "student", "Student role verified");

    // Permission checks
    TEST_ASSERT(AuthManager::check_role(admin_ctx, {"admin"}, err), "Admin authorized for admin action");
    TEST_ASSERT(!AuthManager::check_role(teacher_ctx, {"admin"}, err), "Teacher forbidden from admin action");
    TEST_ASSERT(AuthManager::check_role(teacher_ctx, {"teacher", "admin"}, err), "Teacher authorized for marks entry");
    TEST_ASSERT(!AuthManager::check_role(student_ctx, {"teacher", "admin"}, err), "Student forbidden from marks entry");

    return true;
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << " STUDENT RESULT MANAGEMENT SYSTEM - TEST SUITE           \n";
    std::cout << " Madhav Institute of Technology & Science, Gwalior       \n";
    std::cout << "=========================================================\n";

    bool ok = true;
    ok = ok && test_calculation_edge_cases();
    ok = ok && test_database_crud();
    ok = ok && test_auth_and_roles();

    std::cout << "\n=========================================================\n";
    if (ok) {
        std::cout << " ALL 20 TESTS AND EDGE CASES PASSED SUCCESSFULLY! (100%)\n";
        std::cout << "=========================================================\n";
        return 0;
    } else {
        std::cerr << " SOME TESTS FAILED.\n";
        std::cout << "=========================================================\n";
        return 1;
    }
}
