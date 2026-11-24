// ============================================================================
// Supabase Authentication & Role-Based Authorization
// Student Result Management System - MITS Gwalior
// ============================================================================

#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <vector>
#include "database.h"
#include "crow.h"
#include "nlohmann/json.hpp"

struct AuthContext {
    bool is_authenticated = false;
    std::string user_id;
    std::string email;
    std::string role; // "admin", "teacher", "student"
    std::string name;
    long long exp = 0;
};

class AuthManager {
public:
    static AuthContext verify_token(const std::string& auth_header, Database& db, std::string& err_msg);
    static bool check_role(const AuthContext& ctx, const std::vector<std::string>& allowed_roles, std::string& err_msg);
    
    // API Route Handler
    static nlohmann::json handle_get_me(const AuthContext& ctx, Database& db, int& status_code);

    // JWT Helper Utilities
    static std::string base64url_decode(const std::string& input);
    static bool parse_jwt_payload(const std::string& token, nlohmann::json& out_payload, std::string& err_msg);
};

#endif // AUTH_H
