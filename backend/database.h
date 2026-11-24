// ============================================================================
// Database Management Layer (Supabase PostgreSQL & Local Persistence)
// Student Result Management System - MITS Gwalior
// ============================================================================

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <mutex>
#include "nlohmann/json.hpp"

struct StudentRecord {
    std::string student_id;
    std::string name;
    std::string class_name;
    std::string roll_no;
    std::string auth_user_id;
    std::string created_at;

    nlohmann::json to_json() const;
    static StudentRecord from_json(const nlohmann::json& j);
};

struct TeacherRecord {
    std::string teacher_id;
    std::string name;
    std::string subject;
    std::string auth_user_id;
    std::string created_at;

    nlohmann::json to_json() const;
    static TeacherRecord from_json(const nlohmann::json& j);
};

struct SubjectRecord {
    std::string subject_id;
    std::string subject_name;
    std::string created_at;

    nlohmann::json to_json() const;
    static SubjectRecord from_json(const nlohmann::json& j);
};

struct MarkRecord {
    std::string marks_id;
    std::string student_id;
    std::string student_name;
    std::string subject_id;
    std::string subject_name;
    double marks_obtained = 0.0;
    double maximum_marks = 100.0;
    std::string created_at;
    std::string updated_at;

    nlohmann::json to_json() const;
    static MarkRecord from_json(const nlohmann::json& j);
};

struct ResultRecord {
    std::string result_id;
    std::string student_id;
    std::string student_name;
    std::string roll_no;
    std::string class_name;
    double total_marks = 0.0;
    double maximum_total_marks = 0.0;
    double percentage = 0.0;
    std::string grade;
    std::string result_status; // "Pass" or "Fail"
    std::string updated_at;

    nlohmann::json to_json() const;
    static ResultRecord from_json(const nlohmann::json& j);
};

struct UserProfile {
    std::string id;
    std::string email;
    std::string role; // "admin", "teacher", "student"
    std::string name;
    std::string created_at;

    nlohmann::json to_json() const;
    static UserProfile from_json(const nlohmann::json& j);
};

class Database {
private:
    std::string db_url;
    std::string storage_path;
    std::mutex db_mutex;
    bool using_postgres = false;

    // In-memory data store for local fallback/caching
    std::vector<StudentRecord> students;
    std::vector<TeacherRecord> teachers;
    std::vector<SubjectRecord> subjects;
    std::vector<MarkRecord> marks;
    std::vector<ResultRecord> results;
    std::vector<UserProfile> profiles;

public:
    Database();
    ~Database();

    bool init(const std::string& env_db_url, const std::string& local_path = "data/database.json");
    void load_env_file(const std::string& filepath = ".env");

    // Student Operations
    std::vector<StudentRecord> get_students(const std::string& search_query = "");
    bool get_student(const std::string& student_id, StudentRecord& out);
    bool get_student_by_auth_id(const std::string& auth_id, StudentRecord& out);
    bool add_student(const StudentRecord& s, std::string& err_msg);
    bool update_student(const std::string& student_id, const StudentRecord& s, std::string& err_msg);
    bool delete_student(const std::string& student_id, std::string& err_msg);

    // Teacher Operations
    std::vector<TeacherRecord> get_teachers(const std::string& search_query = "");
    bool get_teacher(const std::string& teacher_id, TeacherRecord& out);
    bool get_teacher_by_auth_id(const std::string& auth_id, TeacherRecord& out);
    bool add_teacher(const TeacherRecord& t, std::string& err_msg);
    bool update_teacher(const std::string& teacher_id, const TeacherRecord& t, std::string& err_msg);
    bool delete_teacher(const std::string& teacher_id, std::string& err_msg);

    // Subject Operations
    std::vector<SubjectRecord> get_subjects();
    bool get_subject(const std::string& subject_id, SubjectRecord& out);
    bool add_subject(const SubjectRecord& sub, std::string& err_msg);
    bool update_subject(const std::string& subject_id, const SubjectRecord& sub, std::string& err_msg);
    bool delete_subject(const std::string& subject_id, std::string& err_msg);
    bool subject_has_marks(const std::string& subject_id);

    // Marks Operations
    std::vector<MarkRecord> get_marks(const std::string& student_id_filter = "");
    bool get_mark(const std::string& marks_id, MarkRecord& out);
    bool add_or_update_mark(const MarkRecord& m, std::string& err_msg);
    bool delete_mark(const std::string& marks_id, std::string& err_msg);

    // Results Operations
    std::vector<ResultRecord> get_results();
    bool get_result_by_student(const std::string& student_id, ResultRecord& out);
    bool save_result(const ResultRecord& r);

    // Profile & Auth Operations
    bool get_profile(const std::string& auth_id, UserProfile& out);
    bool get_profile_by_email(const std::string& email, UserProfile& out);
    bool add_or_update_profile(const UserProfile& p);

    // Dashboard Statistics
    nlohmann::json get_dashboard_stats();

private:
    void seed_default_mits_data();
    void persist_local();
    void load_local();
    std::string get_current_timestamp();
};

#endif // DATABASE_H
