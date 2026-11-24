// ============================================================================
// STUDENT RESULT MANAGEMENT SYSTEM - BACKEND REST API
// Institution: Madhav Institute of Technology & Science, Gwalior
// Academic Session: 2026-27
// Primary Framework: Crow C++ / C++17
// ============================================================================

#include <iostream>
#include <cstdlib>
#include "crow.h"
#include "nlohmann/json.hpp"
#include "database.h"
#include "auth.h"
#include "student.h"
#include "teacher.h"
#include "subject.h"
#include "marks.h"
#include "result.h"

int main() {
    // 1. Initialize Database & Configuration
    Database db;
    db.load_env_file(".env");

    const char* env_db_url = std::getenv("SUPABASE_DB_URL");
    std::string db_url = env_db_url ? env_db_url : "";

    const char* env_port = std::getenv("PORT");
    int port = env_port ? std::atoi(env_port) : 18080;
    if (port <= 0) port = 18080;

    if (!db.init(db_url, "data/database.json")) {
        std::cerr << "[Error] Failed to initialize database subsystem.\n";
        return 1;
    }

    // 2. Setup Crow Application
    crow::SimpleApp app;

    // Helper lambda for JSON parsing
    auto parse_body = [](const crow::request& req, nlohmann::json& out) -> bool {
        if (req.body.empty()) {
            out = nlohmann::json::object();
            return true;
        }
        try {
            out = nlohmann::json::parse(req.body);
            return true;
        } catch (...) {
            return false;
        }
    };

    // Helper lambda to respond with JSON
    auto json_resp = [](int code, const nlohmann::json& data) {
        crow::response res(code);
        res.set_json(data);
        return res;
    };

    // Helper lambda to respond with error JSON
    auto err_resp = [](int code, const std::string& msg) {
        nlohmann::json j = nlohmann::json::object();
        j["success"] = false;
        j["error"] = msg;
        crow::response res(code);
        res.set_json(j);
        return res;
    };

    // ------------------------------------------------------------------------
    // System Health & Information
    // ------------------------------------------------------------------------
    CROW_ROUTE(app, "/")
        .methods(crow::HTTPMethod::GET)
        ([](const crow::request&) {
            nlohmann::json j = nlohmann::json::object();
            j["system"] = "Student Result Management System";
            j["institution"] = "Madhav Institute of Technology & Science, Gwalior";
            j["academic_session"] = "2026-27";
            j["status"] = "Online";
            j["backend"] = "C++17 Crow Microframework";
            return crow::response(200, j.dump());
        });

    // ------------------------------------------------------------------------
    // Dashboard Statistics
    // ------------------------------------------------------------------------
    CROW_ROUTE(app, "/api/stats")
        .methods(crow::HTTPMethod::GET)
        ([&db, &json_resp](const crow::request&) {
            nlohmann::json stats = db.get_dashboard_stats();
            return json_resp(200, stats);
        });

    // ------------------------------------------------------------------------
    // Authentication: Current User Profile
    // ------------------------------------------------------------------------
    CROW_ROUTE(app, "/api/auth/me")
        .methods(crow::HTTPMethod::GET)
        ([&db, &json_resp](const crow::request& req) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            int code = 200;
            nlohmann::json res = AuthManager::handle_get_me(ctx, db, code);
            return json_resp(code, res);
        });

    // ------------------------------------------------------------------------
    // Student Endpoints
    // ------------------------------------------------------------------------
    CROW_ROUTE(app, "/api/students")
        .methods(crow::HTTPMethod::GET)
        ([&db, &json_resp, &err_resp](const crow::request& req) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            // Both Admin and Teacher can view student list
            if (!AuthManager::check_role(ctx, {"admin", "teacher"}, err)) {
                return err_resp(403, err);
            }
            int code = 200;
            nlohmann::json res = StudentController::handle_get_all(db, req, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/students")
        .methods(crow::HTTPMethod::POST)
        ([&db, &json_resp, &err_resp, &parse_body](const crow::request& req) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"admin"}, err)) {
                return err_resp(403, err);
            }
            nlohmann::json body;
            if (!parse_body(req, body)) {
                return err_resp(400, "Invalid JSON in request body");
            }
            int code = 201;
            nlohmann::json res = StudentController::handle_create(db, body, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/students/<string>")
        .methods(crow::HTTPMethod::GET)
        ([&db, &json_resp, &err_resp](const crow::request& req, const std::string& id) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!ctx.is_authenticated) {
                return err_resp(401, "Authentication required");
            }
            // Student can only view their own profile
            if (ctx.role == "student") {
                StudentRecord s;
                if (!db.get_student_by_auth_id(ctx.user_id, s) || s.student_id != id) {
                    return err_resp(403, "Students can only access their own record");
                }
            }
            int code = 200;
            nlohmann::json res = StudentController::handle_get_by_id(db, id, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/students/<string>")
        .methods(crow::HTTPMethod::PUT)
        ([&db, &json_resp, &err_resp, &parse_body](const crow::request& req, const std::string& id) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"admin"}, err)) {
                return err_resp(403, err);
            }
            nlohmann::json body;
            if (!parse_body(req, body)) {
                return err_resp(400, "Invalid JSON in request body");
            }
            int code = 200;
            nlohmann::json res = StudentController::handle_update(db, id, body, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/students/<string>")
        .methods(crow::HTTPMethod::DELETE)
        ([&db, &json_resp, &err_resp](const crow::request& req, const std::string& id) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"admin"}, err)) {
                return err_resp(403, err);
            }
            int code = 200;
            nlohmann::json res = StudentController::handle_delete(db, id, code);
            return json_resp(code, res);
        });

    // ------------------------------------------------------------------------
    // Teacher Endpoints
    // ------------------------------------------------------------------------
    CROW_ROUTE(app, "/api/teachers")
        .methods(crow::HTTPMethod::GET)
        ([&db, &json_resp, &err_resp](const crow::request& req) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!ctx.is_authenticated) {
                return err_resp(401, "Authentication required");
            }
            int code = 200;
            nlohmann::json res = TeacherController::handle_get_all(db, req, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/teachers")
        .methods(crow::HTTPMethod::POST)
        ([&db, &json_resp, &err_resp, &parse_body](const crow::request& req) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"admin"}, err)) {
                return err_resp(403, err);
            }
            nlohmann::json body;
            if (!parse_body(req, body)) {
                return err_resp(400, "Invalid JSON in request body");
            }
            int code = 201;
            nlohmann::json res = TeacherController::handle_create(db, body, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/teachers/<string>")
        .methods(crow::HTTPMethod::PUT)
        ([&db, &json_resp, &err_resp, &parse_body](const crow::request& req, const std::string& id) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"admin"}, err)) {
                return err_resp(403, err);
            }
            nlohmann::json body;
            if (!parse_body(req, body)) {
                return err_resp(400, "Invalid JSON in request body");
            }
            int code = 200;
            nlohmann::json res = TeacherController::handle_update(db, id, body, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/teachers/<string>")
        .methods(crow::HTTPMethod::DELETE)
        ([&db, &json_resp, &err_resp](const crow::request& req, const std::string& id) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"admin"}, err)) {
                return err_resp(403, err);
            }
            int code = 200;
            nlohmann::json res = TeacherController::handle_delete(db, id, code);
            return json_resp(code, res);
        });

    // ------------------------------------------------------------------------
    // Subject Endpoints
    // ------------------------------------------------------------------------
    CROW_ROUTE(app, "/api/subjects")
        .methods(crow::HTTPMethod::GET)
        ([&db, &json_resp, &err_resp](const crow::request& req) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!ctx.is_authenticated) {
                return err_resp(401, "Authentication required");
            }
            int code = 200;
            nlohmann::json res = SubjectController::handle_get_all(db, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/subjects")
        .methods(crow::HTTPMethod::POST)
        ([&db, &json_resp, &err_resp, &parse_body](const crow::request& req) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"admin"}, err)) {
                return err_resp(403, err);
            }
            nlohmann::json body;
            if (!parse_body(req, body)) {
                return err_resp(400, "Invalid JSON in request body");
            }
            int code = 201;
            nlohmann::json res = SubjectController::handle_create(db, body, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/subjects/<string>")
        .methods(crow::HTTPMethod::PUT)
        ([&db, &json_resp, &err_resp, &parse_body](const crow::request& req, const std::string& id) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"admin"}, err)) {
                return err_resp(403, err);
            }
            nlohmann::json body;
            if (!parse_body(req, body)) {
                return err_resp(400, "Invalid JSON in request body");
            }
            int code = 200;
            nlohmann::json res = SubjectController::handle_update(db, id, body, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/subjects/<string>")
        .methods(crow::HTTPMethod::DELETE)
        ([&db, &json_resp, &err_resp](const crow::request& req, const std::string& id) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"admin"}, err)) {
                return err_resp(403, err);
            }
            int code = 200;
            nlohmann::json res = SubjectController::handle_delete(db, id, code);
            return json_resp(code, res);
        });

    // ------------------------------------------------------------------------
    // Marks Endpoints
    // ------------------------------------------------------------------------
    CROW_ROUTE(app, "/api/marks")
        .methods(crow::HTTPMethod::GET)
        ([&db, &json_resp, &err_resp](const crow::request& req) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!ctx.is_authenticated) {
                return err_resp(401, "Authentication required");
            }
            // If student, can only view their own marks
            if (ctx.role == "student") {
                StudentRecord s;
                if (!db.get_student_by_auth_id(ctx.user_id, s)) {
                    std::vector<StudentRecord> stus = db.get_students();
                    if (!stus.empty()) s = stus[0];
                }
                std::vector<MarkRecord> m_list = db.get_marks(s.student_id);
                nlohmann::json arr = nlohmann::json::array();
                for (const auto& m : m_list) arr.push_back(m.to_json());
                nlohmann::json resp_j = nlohmann::json::object();
                resp_j["success"] = true;
                resp_j["marks"] = arr;
                return json_resp(200, resp_j);
            }
            int code = 200;
            nlohmann::json res = MarksController::handle_get_all(db, req, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/marks")
        .methods(crow::HTTPMethod::POST)
        ([&db, &json_resp, &err_resp, &parse_body](const crow::request& req) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            // Teachers and Admins can enter marks
            if (!AuthManager::check_role(ctx, {"teacher", "admin"}, err)) {
                return err_resp(403, err);
            }
            nlohmann::json body;
            if (!parse_body(req, body)) {
                return err_resp(400, "Invalid JSON in request body");
            }
            int code = 201;
            nlohmann::json res = MarksController::handle_save(db, body, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/marks/<string>")
        .methods(crow::HTTPMethod::PUT)
        ([&db, &json_resp, &err_resp, &parse_body](const crow::request& req, const std::string& id) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"teacher", "admin"}, err)) {
                return err_resp(403, err);
            }
            nlohmann::json body;
            if (!parse_body(req, body)) {
                return err_resp(400, "Invalid JSON in request body");
            }
            int code = 200;
            nlohmann::json res = MarksController::handle_update(db, id, body, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/marks/<string>")
        .methods(crow::HTTPMethod::DELETE)
        ([&db, &json_resp, &err_resp](const crow::request& req, const std::string& id) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"admin"}, err)) {
                return err_resp(403, err);
            }
            int code = 200;
            nlohmann::json res = MarksController::handle_delete(db, id, code);
            return json_resp(code, res);
        });

    // ------------------------------------------------------------------------
    // Results Endpoints (Driven by C++ Calculation Engine)
    // ------------------------------------------------------------------------
    CROW_ROUTE(app, "/api/results/calculate")
        .methods(crow::HTTPMethod::POST)
        ([&db, &json_resp, &err_resp, &parse_body](const crow::request& req) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            // Teachers and Admins can trigger result calculation
            if (!AuthManager::check_role(ctx, {"teacher", "admin"}, err)) {
                return err_resp(403, err);
            }
            nlohmann::json body;
            parse_body(req, body);
            int code = 200;
            nlohmann::json res = ResultEngine::handle_calculate(db, body, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/results")
        .methods(crow::HTTPMethod::GET)
        ([&db, &json_resp, &err_resp](const crow::request& req) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!AuthManager::check_role(ctx, {"teacher", "admin"}, err)) {
                return err_resp(403, err);
            }
            int code = 200;
            nlohmann::json res = ResultEngine::handle_get_all_results(db, code);
            return json_resp(code, res);
        });

    CROW_ROUTE(app, "/api/results/<string>")
        .methods(crow::HTTPMethod::GET)
        ([&db, &json_resp, &err_resp](const crow::request& req, const std::string& student_id) {
            std::string auth_header = req.get_header_value("Authorization");
            std::string err;
            AuthContext ctx = AuthManager::verify_token(auth_header, db, err);
            if (!ctx.is_authenticated) {
                return err_resp(401, "Authentication required");
            }
            // Students can only view their own result
            if (ctx.role == "student") {
                StudentRecord s;
                if (!db.get_student_by_auth_id(ctx.user_id, s)) {
                    std::vector<StudentRecord> stus = db.get_students();
                    if (!stus.empty()) s = stus[0];
                }
                if (s.student_id != student_id) {
                    return err_resp(403, "Access denied: You may only view your own academic result");
                }
            }
            int code = 200;
            nlohmann::json res = ResultEngine::handle_get_student_result(db, student_id, code);
            return json_resp(code, res);
        });

    // ------------------------------------------------------------------------
    // Start Server
    // ------------------------------------------------------------------------
    app.port(port).multithreaded().run();
    return 0;
}
