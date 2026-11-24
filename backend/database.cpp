// ============================================================================
// Database Management Implementation
// Student Result Management System - MITS Gwalior
// ============================================================================

#include "database.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>

// JSON Serialization implementations
nlohmann::json StudentRecord::to_json() const {
    nlohmann::json j;
    j["student_id"] = student_id;
    j["name"] = name;
    j["class"] = class_name;
    j["roll_no"] = roll_no;
    j["auth_user_id"] = auth_user_id;
    j["created_at"] = created_at;
    return j;
}

StudentRecord StudentRecord::from_json(const nlohmann::json& j) {
    StudentRecord s;
    s.student_id = j.value("student_id", "");
    s.name = j.value("name", "");
    s.class_name = j.value("class", "");
    s.roll_no = j.value("roll_no", "");
    s.auth_user_id = j.value("auth_user_id", "");
    s.created_at = j.value("created_at", "");
    return s;
}

nlohmann::json TeacherRecord::to_json() const {
    nlohmann::json j;
    j["teacher_id"] = teacher_id;
    j["name"] = name;
    j["subject"] = subject;
    j["auth_user_id"] = auth_user_id;
    j["created_at"] = created_at;
    return j;
}

TeacherRecord TeacherRecord::from_json(const nlohmann::json& j) {
    TeacherRecord t;
    t.teacher_id = j.value("teacher_id", "");
    t.name = j.value("name", "");
    t.subject = j.value("subject", "");
    t.auth_user_id = j.value("auth_user_id", "");
    t.created_at = j.value("created_at", "");
    return t;
}

nlohmann::json SubjectRecord::to_json() const {
    nlohmann::json j;
    j["subject_id"] = subject_id;
    j["subject_name"] = subject_name;
    j["created_at"] = created_at;
    return j;
}

SubjectRecord SubjectRecord::from_json(const nlohmann::json& j) {
    SubjectRecord s;
    s.subject_id = j.value("subject_id", "");
    s.subject_name = j.value("subject_name", "");
    s.created_at = j.value("created_at", "");
    return s;
}

nlohmann::json MarkRecord::to_json() const {
    nlohmann::json j;
    j["marks_id"] = marks_id;
    j["student_id"] = student_id;
    j["student_name"] = student_name;
    j["subject_id"] = subject_id;
    j["subject_name"] = subject_name;
    j["marks_obtained"] = marks_obtained;
    j["maximum_marks"] = maximum_marks;
    j["created_at"] = created_at;
    j["updated_at"] = updated_at;
    return j;
}

MarkRecord MarkRecord::from_json(const nlohmann::json& j) {
    MarkRecord m;
    m.marks_id = j.value("marks_id", "");
    m.student_id = j.value("student_id", "");
    m.student_name = j.value("student_name", "");
    m.subject_id = j.value("subject_id", "");
    m.subject_name = j.value("subject_name", "");
    m.marks_obtained = j.value("marks_obtained", 0.0);
    m.maximum_marks = j.value("maximum_marks", 100.0);
    m.created_at = j.value("created_at", "");
    m.updated_at = j.value("updated_at", "");
    return m;
}

nlohmann::json ResultRecord::to_json() const {
    nlohmann::json j;
    j["result_id"] = result_id;
    j["student_id"] = student_id;
    j["student_name"] = student_name;
    j["roll_no"] = roll_no;
    j["class"] = class_name;
    j["total_marks"] = total_marks;
    j["maximum_total_marks"] = maximum_total_marks;
    j["percentage"] = percentage;
    j["grade"] = grade;
    j["result_status"] = result_status;
    j["updated_at"] = updated_at;
    return j;
}

ResultRecord ResultRecord::from_json(const nlohmann::json& j) {
    ResultRecord r;
    r.result_id = j.value("result_id", "");
    r.student_id = j.value("student_id", "");
    r.student_name = j.value("student_name", "");
    r.roll_no = j.value("roll_no", "");
    r.class_name = j.value("class", "");
    r.total_marks = j.value("total_marks", 0.0);
    r.maximum_total_marks = j.value("maximum_total_marks", 0.0);
    r.percentage = j.value("percentage", 0.0);
    r.grade = j.value("grade", "");
    r.result_status = j.value("result_status", "");
    r.updated_at = j.value("updated_at", "");
    return r;
}

nlohmann::json UserProfile::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["email"] = email;
    j["role"] = role;
    j["name"] = name;
    j["created_at"] = created_at;
    return j;
}

UserProfile UserProfile::from_json(const nlohmann::json& j) {
    UserProfile p;
    p.id = j.value("id", "");
    p.email = j.value("email", "");
    p.role = j.value("role", "student");
    p.name = j.value("name", "");
    p.created_at = j.value("created_at", "");
    return p;
}

// Helper: Case-insensitive substring check
static bool contains_ignore_case(const std::string& str, const std::string& query) {
    if (query.empty()) return true;
    auto it = std::search(
        str.begin(), str.end(),
        query.begin(), query.end(),
        [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
    );
    return it != str.end();
}

Database::Database() = default;
Database::~Database() = default;

void Database::load_env_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            val.erase(0, val.find_first_not_of(" \t\r\n"));
            val.erase(val.find_last_not_of(" \t\r\n") + 1);
            if (!val.empty() && (val.front() == '"' || val.front() == '\'')) val.erase(0, 1);
            if (!val.empty() && (val.back() == '"' || val.back() == '\'')) val.pop_back();
            setenv(key.c_str(), val.c_str(), 0); // don't overwrite if already set
        }
    }
}

std::string Database::get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32)
    gmtime_s(&tm_buf, &now_c);
#else
    gmtime_r(&now_c, &tm_buf);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

bool Database::init(const std::string& env_db_url, const std::string& local_path) {
    std::lock_guard<std::mutex> lock(db_mutex);
    db_url = env_db_url;
    storage_path = local_path;

    if (!db_url.empty() && db_url.find("postgresql://") != std::string::npos) {
        std::cout << "[Database] Supabase PostgreSQL connection configured: " << db_url.substr(0, 20) << "...\n";
        using_postgres = true;
    } else {
        std::cout << "[Database] Local persistent database mode active (" << storage_path << ")\n";
        using_postgres = false;
    }

    // Load data from file if exists; otherwise seed with MITS initial data
    std::ifstream check_file(storage_path);
    if (check_file.good()) {
        check_file.close();
        load_local();
    } else {
        seed_default_mits_data();
        persist_local();
    }
    return true;
}

void Database::seed_default_mits_data() {
    std::string now = get_current_timestamp();

    // 1. Subjects
    subjects = {
        {"SUB101", "Object Oriented Programming with C++", now},
        {"SUB102", "Database Management Systems", now},
        {"SUB103", "Data Structures & Algorithms", now},
        {"SUB104", "Computer Networks", now},
        {"SUB105", "Discrete Mathematics", now}
    };

    // 2. Students
    students = {
        {"STU001", "Aarav Sharma", "B.Tech CSE III Sem", "0901CS241001", "auth-student-001", now},
        {"STU002", "Priya Patel", "B.Tech CSE III Sem", "0901CS241002", "", now},
        {"STU003", "Rohan Gupta", "B.Tech CSE III Sem", "0901CS241003", "", now},
        {"STU004", "Ananya Singh", "B.Tech CSE III Sem", "0901CS241004", "", now},
        {"STU005", "Vikramaditya Verma", "B.Tech CSE III Sem", "0901CS241005", "", now}
    };

    // 3. Teachers
    teachers = {
        {"TCH001", "Dr. Rajesh Kumar Sharma", "Object Oriented Programming with C++", "auth-teacher-001", now},
        {"TCH002", "Prof. Sunita Verma", "Database Management Systems", "", now},
        {"TCH003", "Dr. Amit Dixit", "Data Structures & Algorithms", "", now}
    };

    // 4. Marks for STU001 (Aarav Sharma)
    marks = {
        {"MRK001", "STU001", "Aarav Sharma", "SUB101", "Object Oriented Programming with C++", 94.0, 100.0, now, now},
        {"MRK002", "STU001", "Aarav Sharma", "SUB102", "Database Management Systems", 91.0, 100.0, now, now},
        {"MRK003", "STU001", "Aarav Sharma", "SUB103", "Data Structures & Algorithms", 95.0, 100.0, now, now},
        {"MRK004", "STU001", "Aarav Sharma", "SUB104", "Computer Networks", 88.0, 100.0, now, now},
        {"MRK005", "STU001", "Aarav Sharma", "SUB105", "Discrete Mathematics", 92.0, 100.0, now, now},

        {"MRK006", "STU002", "Priya Patel", "SUB101", "Object Oriented Programming with C++", 82.0, 100.0, now, now},
        {"MRK007", "STU002", "Priya Patel", "SUB102", "Database Management Systems", 85.0, 100.0, now, now},
        {"MRK008", "STU002", "Priya Patel", "SUB103", "Data Structures & Algorithms", 88.0, 100.0, now, now},
        {"MRK009", "STU002", "Priya Patel", "SUB104", "Computer Networks", 79.0, 100.0, now, now},
        {"MRK010", "STU002", "Priya Patel", "SUB105", "Discrete Mathematics", 81.0, 100.0, now, now},

        {"MRK011", "STU003", "Rohan Gupta", "SUB101", "Object Oriented Programming with C++", 72.0, 100.0, now, now},
        {"MRK012", "STU003", "Rohan Gupta", "SUB102", "Database Management Systems", 74.0, 100.0, now, now},
        {"MRK013", "STU003", "Rohan Gupta", "SUB103", "Data Structures & Algorithms", 70.0, 100.0, now, now},
        {"MRK014", "STU003", "Rohan Gupta", "SUB104", "Computer Networks", 75.0, 100.0, now, now},
        {"MRK015", "STU003", "Rohan Gupta", "SUB105", "Discrete Mathematics", 71.0, 100.0, now, now},

        {"MRK016", "STU004", "Ananya Singh", "SUB101", "Object Oriented Programming with C++", 65.0, 100.0, now, now},
        {"MRK017", "STU004", "Ananya Singh", "SUB102", "Database Management Systems", 62.0, 100.0, now, now},
        {"MRK018", "STU004", "Ananya Singh", "SUB103", "Data Structures & Algorithms", 68.0, 100.0, now, now},
        {"MRK019", "STU004", "Ananya Singh", "SUB104", "Computer Networks", 61.0, 100.0, now, now},
        {"MRK020", "STU004", "Ananya Singh", "SUB105", "Discrete Mathematics", 64.0, 100.0, now, now}
    };

    // 5. Results
    results = {
        {"RES001", "STU001", "Aarav Sharma", "0901CS241001", "B.Tech CSE III Sem", 460.0, 500.0, 92.00, "A+", "Pass", now},
        {"RES002", "STU002", "Priya Patel", "0901CS241002", "B.Tech CSE III Sem", 415.0, 500.0, 83.00, "A", "Pass", now},
        {"RES003", "STU003", "Rohan Gupta", "0901CS241003", "B.Tech CSE III Sem", 362.0, 500.0, 72.40, "B", "Pass", now},
        {"RES004", "STU004", "Ananya Singh", "0901CS241004", "B.Tech CSE III Sem", 320.0, 500.0, 64.00, "C", "Pass", now}
    };

    // 6. Profiles for role testing
    profiles = {
        {"auth-admin-001", "admin@mits.ac.in", "admin", "System Administrator", now},
        {"auth-teacher-001", "teacher@mits.ac.in", "teacher", "Dr. Rajesh Kumar Sharma", now},
        {"auth-student-001", "student@mits.ac.in", "student", "Aarav Sharma", now}
    };
}

void Database::persist_local() {
    nlohmann::json root = nlohmann::json::object();

    nlohmann::json s_arr = nlohmann::json::array();
    for (const auto& s : students) s_arr.push_back(s.to_json());
    root["students"] = s_arr;

    nlohmann::json t_arr = nlohmann::json::array();
    for (const auto& t : teachers) t_arr.push_back(t.to_json());
    root["teachers"] = t_arr;

    nlohmann::json sub_arr = nlohmann::json::array();
    for (const auto& sub : subjects) sub_arr.push_back(sub.to_json());
    root["subjects"] = sub_arr;

    nlohmann::json m_arr = nlohmann::json::array();
    for (const auto& m : marks) m_arr.push_back(m.to_json());
    root["marks"] = m_arr;

    nlohmann::json r_arr = nlohmann::json::array();
    for (const auto& r : results) r_arr.push_back(r.to_json());
    root["results"] = r_arr;

    nlohmann::json p_arr = nlohmann::json::array();
    for (const auto& p : profiles) p_arr.push_back(p.to_json());
    root["profiles"] = p_arr;

    std::ofstream out(storage_path);
    if (out.is_open()) {
        out << root.dump(2);
    }
}

void Database::load_local() {
    std::ifstream in(storage_path);
    if (!in.is_open()) return;
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string content = buffer.str();
    if (content.empty()) return;

    nlohmann::json root = nlohmann::json::parse(content);
    students.clear();
    teachers.clear();
    subjects.clear();
    marks.clear();
    results.clear();
    profiles.clear();

    if (root.contains("students")) {
        for (const auto& j : root["students"]) students.push_back(StudentRecord::from_json(j));
    }
    if (root.contains("teachers")) {
        for (const auto& j : root["teachers"]) teachers.push_back(TeacherRecord::from_json(j));
    }
    if (root.contains("subjects")) {
        for (const auto& j : root["subjects"]) subjects.push_back(SubjectRecord::from_json(j));
    }
    if (root.contains("marks")) {
        for (const auto& j : root["marks"]) marks.push_back(MarkRecord::from_json(j));
    }
    if (root.contains("results")) {
        for (const auto& j : root["results"]) results.push_back(ResultRecord::from_json(j));
    }
    if (root.contains("profiles")) {
        for (const auto& j : root["profiles"]) profiles.push_back(UserProfile::from_json(j));
    }
}

// ----------------------------------------------------------------------------
// Student Methods
// ----------------------------------------------------------------------------
std::vector<StudentRecord> Database::get_students(const std::string& query) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (query.empty()) return students;
    std::vector<StudentRecord> res;
    for (const auto& s : students) {
        if (contains_ignore_case(s.name, query) ||
            contains_ignore_case(s.student_id, query) ||
            contains_ignore_case(s.roll_no, query) ||
            contains_ignore_case(s.class_name, query)) {
            res.push_back(s);
        }
    }
    return res;
}

bool Database::get_student(const std::string& student_id, StudentRecord& out) {
    std::lock_guard<std::mutex> lock(db_mutex);
    for (const auto& s : students) {
        if (s.student_id == student_id) {
            out = s;
            return true;
        }
    }
    return false;
}

bool Database::get_student_by_auth_id(const std::string& auth_id, StudentRecord& out) {
    std::lock_guard<std::mutex> lock(db_mutex);
    for (const auto& s : students) {
        if (s.auth_user_id == auth_id) {
            out = s;
            return true;
        }
    }
    return false;
}

bool Database::add_student(const StudentRecord& s, std::string& err_msg) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (s.student_id.empty() || s.name.empty() || s.roll_no.empty() || s.class_name.empty()) {
        err_msg = "All fields (Student ID, Name, Roll No, Class) are required";
        return false;
    }
    for (const auto& ex : students) {
        if (ex.student_id == s.student_id) {
            err_msg = "Student with ID '" + s.student_id + "' already exists";
            return false;
        }
        if (ex.roll_no == s.roll_no) {
            err_msg = "Student with Roll Number '" + s.roll_no + "' already exists";
            return false;
        }
    }
    StudentRecord record = s;
    if (record.created_at.empty()) record.created_at = get_current_timestamp();
    students.push_back(record);
    persist_local();
    return true;
}

bool Database::update_student(const std::string& student_id, const StudentRecord& s, std::string& err_msg) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = std::find_if(students.begin(), students.end(),
                           [&](const StudentRecord& r) { return r.student_id == student_id; });
    if (it == students.end()) {
        err_msg = "Student not found with ID: " + student_id;
        return false;
    }
    // Check if new roll_no conflicts with another student
    for (const auto& ex : students) {
        if (ex.student_id != student_id && ex.roll_no == s.roll_no) {
            err_msg = "Roll Number '" + s.roll_no + "' is already assigned to another student";
            return false;
        }
    }

    it->name = s.name.empty() ? it->name : s.name;
    it->class_name = s.class_name.empty() ? it->class_name : s.class_name;
    it->roll_no = s.roll_no.empty() ? it->roll_no : s.roll_no;
    if (!s.auth_user_id.empty()) it->auth_user_id = s.auth_user_id;

    // Update student name in marks and results
    for (auto& m : marks) {
        if (m.student_id == student_id) m.student_name = it->name;
    }
    for (auto& r : results) {
        if (r.student_id == student_id) {
            r.student_name = it->name;
            r.roll_no = it->roll_no;
            r.class_name = it->class_name;
        }
    }

    persist_local();
    return true;
}

bool Database::delete_student(const std::string& student_id, std::string& err_msg) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = std::find_if(students.begin(), students.end(),
                           [&](const StudentRecord& r) { return r.student_id == student_id; });
    if (it == students.end()) {
        err_msg = "Student not found with ID: " + student_id;
        return false;
    }
    students.erase(it);

    // Cascade delete marks & results
    marks.erase(std::remove_if(marks.begin(), marks.end(),
                               [&](const MarkRecord& m) { return m.student_id == student_id; }),
                marks.end());
    results.erase(std::remove_if(results.begin(), results.end(),
                                 [&](const ResultRecord& r) { return r.student_id == student_id; }),
                  results.end());

    persist_local();
    return true;
}

// ----------------------------------------------------------------------------
// Teacher Methods
// ----------------------------------------------------------------------------
std::vector<TeacherRecord> Database::get_teachers(const std::string& query) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (query.empty()) return teachers;
    std::vector<TeacherRecord> res;
    for (const auto& t : teachers) {
        if (contains_ignore_case(t.name, query) ||
            contains_ignore_case(t.teacher_id, query) ||
            contains_ignore_case(t.subject, query)) {
            res.push_back(t);
        }
    }
    return res;
}

bool Database::get_teacher(const std::string& teacher_id, TeacherRecord& out) {
    std::lock_guard<std::mutex> lock(db_mutex);
    for (const auto& t : teachers) {
        if (t.teacher_id == teacher_id) {
            out = t;
            return true;
        }
    }
    return false;
}

bool Database::get_teacher_by_auth_id(const std::string& auth_id, TeacherRecord& out) {
    std::lock_guard<std::mutex> lock(db_mutex);
    for (const auto& t : teachers) {
        if (t.auth_user_id == auth_id) {
            out = t;
            return true;
        }
    }
    return false;
}

bool Database::add_teacher(const TeacherRecord& t, std::string& err_msg) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (t.teacher_id.empty() || t.name.empty() || t.subject.empty()) {
        err_msg = "Teacher ID, Name, and Subject are required";
        return false;
    }
    for (const auto& ex : teachers) {
        if (ex.teacher_id == t.teacher_id) {
            err_msg = "Teacher with ID '" + t.teacher_id + "' already exists";
            return false;
        }
    }
    TeacherRecord record = t;
    if (record.created_at.empty()) record.created_at = get_current_timestamp();
    teachers.push_back(record);
    persist_local();
    return true;
}

bool Database::update_teacher(const std::string& teacher_id, const TeacherRecord& t, std::string& err_msg) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = std::find_if(teachers.begin(), teachers.end(),
                           [&](const TeacherRecord& r) { return r.teacher_id == teacher_id; });
    if (it == teachers.end()) {
        err_msg = "Teacher not found with ID: " + teacher_id;
        return false;
    }
    it->name = t.name.empty() ? it->name : t.name;
    it->subject = t.subject.empty() ? it->subject : t.subject;
    if (!t.auth_user_id.empty()) it->auth_user_id = t.auth_user_id;

    persist_local();
    return true;
}

bool Database::delete_teacher(const std::string& teacher_id, std::string& err_msg) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = std::find_if(teachers.begin(), teachers.end(),
                           [&](const TeacherRecord& r) { return r.teacher_id == teacher_id; });
    if (it == teachers.end()) {
        err_msg = "Teacher not found with ID: " + teacher_id;
        return false;
    }
    teachers.erase(it);
    persist_local();
    return true;
}

// ----------------------------------------------------------------------------
// Subject Methods
// ----------------------------------------------------------------------------
std::vector<SubjectRecord> Database::get_subjects() {
    std::lock_guard<std::mutex> lock(db_mutex);
    return subjects;
}

bool Database::get_subject(const std::string& subject_id, SubjectRecord& out) {
    std::lock_guard<std::mutex> lock(db_mutex);
    for (const auto& s : subjects) {
        if (s.subject_id == subject_id) {
            out = s;
            return true;
        }
    }
    return false;
}

bool Database::subject_has_marks(const std::string& subject_id) {
    for (const auto& m : marks) {
        if (m.subject_id == subject_id) return true;
    }
    return false;
}

bool Database::add_subject(const SubjectRecord& sub, std::string& err_msg) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (sub.subject_id.empty() || sub.subject_name.empty()) {
        err_msg = "Subject ID and Subject Name are required";
        return false;
    }
    for (const auto& ex : subjects) {
        if (ex.subject_id == sub.subject_id) {
            err_msg = "Subject ID '" + sub.subject_id + "' already exists";
            return false;
        }
        if (ex.subject_name == sub.subject_name) {
            err_msg = "Subject with name '" + sub.subject_name + "' already exists";
            return false;
        }
    }
    SubjectRecord record = sub;
    if (record.created_at.empty()) record.created_at = get_current_timestamp();
    subjects.push_back(record);
    persist_local();
    return true;
}

bool Database::update_subject(const std::string& subject_id, const SubjectRecord& sub, std::string& err_msg) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = std::find_if(subjects.begin(), subjects.end(),
                           [&](const SubjectRecord& r) { return r.subject_id == subject_id; });
    if (it == subjects.end()) {
        err_msg = "Subject not found with ID: " + subject_id;
        return false;
    }
    it->subject_name = sub.subject_name.empty() ? it->subject_name : sub.subject_name;

    // Update subject name in marks
    for (auto& m : marks) {
        if (m.subject_id == subject_id) m.subject_name = it->subject_name;
    }

    persist_local();
    return true;
}

bool Database::delete_subject(const std::string& subject_id, std::string& err_msg) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = std::find_if(subjects.begin(), subjects.end(),
                           [&](const SubjectRecord& r) { return r.subject_id == subject_id; });
    if (it == subjects.end()) {
        err_msg = "Subject not found with ID: " + subject_id;
        return false;
    }
    if (subject_has_marks(subject_id)) {
        err_msg = "Cannot delete subject: Existing examination marks are recorded for this subject";
        return false;
    }
    subjects.erase(it);
    persist_local();
    return true;
}

// ----------------------------------------------------------------------------
// Marks Methods
// ----------------------------------------------------------------------------
std::vector<MarkRecord> Database::get_marks(const std::string& student_id_filter) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (student_id_filter.empty()) return marks;
    std::vector<MarkRecord> res;
    for (const auto& m : marks) {
        if (m.student_id == student_id_filter) {
            res.push_back(m);
        }
    }
    return res;
}

bool Database::get_mark(const std::string& marks_id, MarkRecord& out) {
    std::lock_guard<std::mutex> lock(db_mutex);
    for (const auto& m : marks) {
        if (m.marks_id == marks_id) {
            out = m;
            return true;
        }
    }
    return false;
}

bool Database::add_or_update_mark(const MarkRecord& m, std::string& err_msg) {
    std::lock_guard<std::mutex> lock(db_mutex);

    // Validation checks
    if (m.student_id.empty() || m.subject_id.empty()) {
        err_msg = "Student ID and Subject ID are required";
        return false;
    }
    if (m.marks_obtained < 0) {
        err_msg = "Marks obtained cannot be negative";
        return false;
    }
    if (m.maximum_marks <= 0) {
        err_msg = "Maximum marks must be greater than zero";
        return false;
    }
    if (m.marks_obtained > m.maximum_marks) {
        err_msg = "Marks obtained cannot exceed maximum marks";
        return false;
    }

    // Lookup Student Name
    std::string s_name = m.student_name;
    auto s_it = std::find_if(students.begin(), students.end(),
                             [&](const StudentRecord& s) { return s.student_id == m.student_id; });
    if (s_it == students.end()) {
        err_msg = "Student with ID '" + m.student_id + "' does not exist";
        return false;
    }
    s_name = s_it->name;

    // Lookup Subject Name
    std::string sub_name = m.subject_name;
    auto sub_it = std::find_if(subjects.begin(), subjects.end(),
                               [&](const SubjectRecord& s) { return s.subject_id == m.subject_id; });
    if (sub_it == subjects.end()) {
        err_msg = "Subject with ID '" + m.subject_id + "' does not exist";
        return false;
    }
    sub_name = sub_it->subject_name;

    std::string now = get_current_timestamp();

    // Check if mark already exists for (student_id, subject_id)
    auto it = std::find_if(marks.begin(), marks.end(), [&](const MarkRecord& item) {
        if (!m.marks_id.empty() && item.marks_id == m.marks_id) return true;
        return item.student_id == m.student_id && item.subject_id == m.subject_id;
    });

    if (it != marks.end()) {
        // Update existing mark
        it->marks_obtained = m.marks_obtained;
        it->maximum_marks = m.maximum_marks;
        it->student_name = s_name;
        it->subject_name = sub_name;
        it->updated_at = now;
    } else {
        // Insert new mark
        MarkRecord new_m = m;
        if (new_m.marks_id.empty()) {
            new_m.marks_id = "MRK" + std::to_string(marks.size() + 101);
        }
        new_m.student_name = s_name;
        new_m.subject_name = sub_name;
        new_m.created_at = now;
        new_m.updated_at = now;
        marks.push_back(new_m);
    }

    persist_local();
    return true;
}

bool Database::delete_mark(const std::string& marks_id, std::string& err_msg) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = std::find_if(marks.begin(), marks.end(),
                           [&](const MarkRecord& r) { return r.marks_id == marks_id; });
    if (it == marks.end()) {
        err_msg = "Marks record not found with ID: " + marks_id;
        return false;
    }
    marks.erase(it);
    persist_local();
    return true;
}

// ----------------------------------------------------------------------------
// Results Methods
// ----------------------------------------------------------------------------
std::vector<ResultRecord> Database::get_results() {
    std::lock_guard<std::mutex> lock(db_mutex);
    return results;
}

bool Database::get_result_by_student(const std::string& student_id, ResultRecord& out) {
    std::lock_guard<std::mutex> lock(db_mutex);
    for (const auto& r : results) {
        if (r.student_id == student_id) {
            out = r;
            return true;
        }
    }
    return false;
}

bool Database::save_result(const ResultRecord& r) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = std::find_if(results.begin(), results.end(),
                           [&](const ResultRecord& ex) { return ex.student_id == r.student_id; });
    if (it != results.end()) {
        *it = r;
    } else {
        results.push_back(r);
    }
    persist_local();
    return true;
}

// ----------------------------------------------------------------------------
// Profile & Auth Methods
// ----------------------------------------------------------------------------
bool Database::get_profile(const std::string& auth_id, UserProfile& out) {
    std::lock_guard<std::mutex> lock(db_mutex);
    for (const auto& p : profiles) {
        if (p.id == auth_id) {
            out = p;
            return true;
        }
    }
    return false;
}

bool Database::get_profile_by_email(const std::string& email, UserProfile& out) {
    std::lock_guard<std::mutex> lock(db_mutex);
    for (const auto& p : profiles) {
        if (p.email == email) {
            out = p;
            return true;
        }
    }
    return false;
}

bool Database::add_or_update_profile(const UserProfile& p) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = std::find_if(profiles.begin(), profiles.end(),
                           [&](const UserProfile& ex) { return ex.id == p.id; });
    if (it != profiles.end()) {
        *it = p;
    } else {
        profiles.push_back(p);
    }
    persist_local();
    return true;
}

// ----------------------------------------------------------------------------
// Dashboard Statistics
// ----------------------------------------------------------------------------
nlohmann::json Database::get_dashboard_stats() {
    std::lock_guard<std::mutex> lock(db_mutex);
    nlohmann::json stats = nlohmann::json::object();

    stats["total_students"] = static_cast<int>(students.size());
    stats["total_teachers"] = static_cast<int>(teachers.size());
    stats["total_subjects"] = static_cast<int>(subjects.size());
    stats["total_results"] = static_cast<int>(results.size());
    stats["total_marks_entered"] = static_cast<int>(marks.size());

    // Recent students (last 5)
    nlohmann::json rec_stu = nlohmann::json::array();
    size_t s_count = std::min(students.size(), size_t(5));
    for (size_t i = 0; i < s_count; ++i) {
        rec_stu.push_back(students[students.size() - 1 - i].to_json());
    }
    stats["recent_students"] = rec_stu;

    // Recent marks (last 5)
    nlohmann::json rec_marks = nlohmann::json::array();
    size_t m_count = std::min(marks.size(), size_t(5));
    for (size_t i = 0; i < m_count; ++i) {
        rec_marks.push_back(marks[marks.size() - 1 - i].to_json());
    }
    stats["recent_marks"] = rec_marks;

    return stats;
}
