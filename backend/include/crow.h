// ============================================================================
// Crow Framework C++17 Header Implementation
// Student Result Management System - MITS Gwalior
// Provides clean, standard Crow routing syntax and built-in HTTP server
// ============================================================================

#ifndef CROW_H
#define CROW_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <regex>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "nlohmann/json.hpp"

namespace crow {

enum class HTTPMethod {
    GET,
    POST,
    PUT,
    DELETE,
    OPTIONS
};

inline std::string method_to_string(HTTPMethod m) {
    switch (m) {
        case HTTPMethod::GET: return "GET";
        case HTTPMethod::POST: return "POST";
        case HTTPMethod::PUT: return "PUT";
        case HTTPMethod::DELETE: return "DELETE";
        case HTTPMethod::OPTIONS: return "OPTIONS";
    }
    return "GET";
}

inline HTTPMethod string_to_method(const std::string& s) {
    if (s == "POST") return HTTPMethod::POST;
    if (s == "PUT") return HTTPMethod::PUT;
    if (s == "DELETE") return HTTPMethod::DELETE;
    if (s == "OPTIONS") return HTTPMethod::OPTIONS;
    return HTTPMethod::GET;
}

// Request URL parameters helper
struct query_string {
    std::map<std::string, std::string> params;

    const char* get(const std::string& key) const {
        auto it = params.find(key);
        if (it != params.end()) {
            return it->second.c_str();
        }
        return nullptr;
    }
};

// Crow Request
struct request {
    std::string method_name;
    HTTPMethod method;
    std::string raw_url;
    std::string url;
    std::string body;
    std::map<std::string, std::string> headers;
    query_string url_params;

    std::string get_header_value(const std::string& key) const {
        std::string lower_key = to_lower(key);
        for (const auto& kv : headers) {
            if (to_lower(kv.first) == lower_key) {
                return kv.second;
            }
        }
        return "";
    }

private:
    static std::string to_lower(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(c));
        return s;
    }
};

// Crow Response
struct response {
    int code = 200;
    std::string body;
    std::map<std::string, std::string> headers;

    response() {
        set_default_headers();
    }

    response(int c) : code(c) {
        set_default_headers();
    }

    response(const std::string& b) : code(200), body(b) {
        set_default_headers();
    }

    response(int c, const std::string& b) : code(c), body(b) {
        set_default_headers();
    }

    void set_header(const std::string& key, const std::string& value) {
        headers[key] = value;
    }

    void set_json(const nlohmann::json& j) {
        set_header("Content-Type", "application/json");
        body = j.dump();
    }

private:
    void set_default_headers() {
        headers["Access-Control-Allow-Origin"] = "*";
        headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
        headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization, X-Requested-With";
        headers["Content-Type"] = "application/json";
    }
};

// Route Definition and Handler
class Route {
public:
    std::string pattern;
    std::vector<HTTPMethod> allowed_methods;
    std::function<response(const request&)> handler_no_args;
    std::function<response(const request&, const std::string&)> handler_one_arg;
    bool has_arg = false;
    std::regex regex_pattern;

    Route(const std::string& p) : pattern(p) {
        allowed_methods.push_back(HTTPMethod::GET); // Default
        init_regex();
    }

    template<typename... Methods>
    Route& methods_impl(HTTPMethod m, Methods... rest) {
        allowed_methods.push_back(m);
        if constexpr (sizeof...(rest) > 0) {
            methods_impl(rest...);
        }
        return *this;
    }

    template<typename... Methods>
    Route& methods(HTTPMethod m, Methods... rest) {
        allowed_methods.clear();
        allowed_methods.push_back(m);
        if constexpr (sizeof...(rest) > 0) {
            methods_impl(rest...);
        }
        return *this;
    }

    void operator()(std::function<response(const request&)> f) {
        handler_no_args = f;
        has_arg = false;
    }

    void operator()(std::function<response(const request&, const std::string&)> f) {
        handler_one_arg = f;
        has_arg = true;
    }

private:
    void init_regex() {
        if (pattern.find("<string>") != std::string::npos) {
            has_arg = true;
            std::string rx = pattern;
            size_t pos = 0;
            while ((pos = rx.find("<string>", pos)) != std::string::npos) {
                rx.replace(pos, 8, "([^/?#]+)");
                pos += 10;
            }
            regex_pattern = std::regex("^" + rx + "$");
        } else {
            regex_pattern = std::regex("^" + pattern + "$");
        }
    }
};

// Crow SimpleApp
class SimpleApp {
private:
    int server_port = 18080;
    std::vector<std::shared_ptr<Route>> routes;
    std::atomic<bool> is_running{false};
    int server_fd = -1;

public:
    SimpleApp() {
        signal(SIGPIPE, SIG_IGN);
    }

    ~SimpleApp() {
        stop();
    }

    SimpleApp& port(int p) {
        server_port = p;
        return *this;
    }

    SimpleApp& multithreaded() {
        return *this;
    }

    Route& route(const std::string& path) {
        auto r = std::make_shared<Route>(path);
        routes.push_back(r);
        return *r;
    }

    void stop() {
        is_running = false;
        if (server_fd >= 0) {
            close(server_fd);
            server_fd = -1;
        }
    }

    void run() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "[Crow Error] Failed to create socket\n";
            return;
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(server_port);

        if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "[Crow Error] Failed to bind to port " << server_port << "\n";
            close(server_fd);
            return;
        }

        if (listen(server_fd, 32) < 0) {
            std::cerr << "[Crow Error] Failed to listen on socket\n";
            close(server_fd);
            return;
        }

        is_running = true;
        std::cout << "=========================================================\n";
        std::cout << "  Student Result Management System - C++ Crow Backend    \n";
        std::cout << "  Madhav Institute of Technology & Science, Gwalior     \n";
        std::cout << "  Server running on http://127.0.0.1:" << server_port << "\n";
        std::cout << "=========================================================\n";

        while (is_running) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                if (!is_running) break;
                continue;
            }

            // Handle connection in a worker thread
            std::thread([this, client_fd]() {
                handle_client(client_fd);
            }).detach();
        }
    }

private:
    void handle_client(int client_fd) {
        std::vector<char> buffer(65536, 0);
        ssize_t bytes_read = read(client_fd, buffer.data(), buffer.size() - 1);
        if (bytes_read <= 0) {
            close(client_fd);
            return;
        }

        std::string raw_request(buffer.data(), bytes_read);
        request req = parse_request(raw_request);

        // Preflight OPTIONS for CORS
        if (req.method == HTTPMethod::OPTIONS) {
            response res(204, "");
            send_response(client_fd, res);
            close(client_fd);
            return;
        }

        response res(404, "{\"error\": \"Endpoint not found\"}");
        bool matched = false;

        for (const auto& r : routes) {
            // Check method match
            bool method_match = false;
            for (auto m : r->allowed_methods) {
                if (m == req.method) {
                    method_match = true;
                    break;
                }
            }
            if (!method_match) continue;

            std::smatch match;
            if (std::regex_match(req.url, match, r->regex_pattern)) {
                matched = true;
                try {
                    if (r->has_arg && match.size() > 1 && r->handler_one_arg) {
                        std::string arg = match[1].str();
                        res = r->handler_one_arg(req, arg);
                    } else if (r->handler_no_args) {
                        res = r->handler_no_args(req);
                    }
                } catch (const std::exception& ex) {
                    res.code = 500;
                    nlohmann::json err = nlohmann::json::object();
                    err["error"] = "Internal server error: " + std::string(ex.what());
                    res.body = err.dump();
                }
                break;
            }
        }

        send_response(client_fd, res);
        close(client_fd);
    }

    request parse_request(const std::string& raw) {
        request req;
        std::istringstream stream(raw);
        std::string line;
        
        // Request line: METHOD /path HTTP/1.1
        if (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            std::istringstream line_stream(line);
            std::string method_str, full_url, http_ver;
            line_stream >> method_str >> full_url >> http_ver;

            req.method_name = method_str;
            req.method = string_to_method(method_str);
            req.raw_url = full_url;

            // Separate path and query string
            size_t query_pos = full_url.find('?');
            if (query_pos != std::string::npos) {
                req.url = full_url.substr(0, query_pos);
                std::string query = full_url.substr(query_pos + 1);
                parse_query(query, req.url_params);
            } else {
                req.url = full_url;
            }
        }

        // Headers
        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) break; // End of headers
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string header_name = line.substr(0, colon);
                std::string header_val = line.substr(colon + 1);
                while (!header_val.empty() && (header_val.front() == ' ' || header_val.front() == '\t')) {
                    header_val.erase(0, 1);
                }
                req.headers[header_name] = header_val;
            }
        }

        // Body
        std::string body_content;
        while (std::getline(stream, line)) {
            body_content += line + "\n";
        }
        if (!body_content.empty() && body_content.back() == '\n') {
            body_content.pop_back();
        }
        req.body = body_content;

        return req;
    }

    void parse_query(const std::string& qs, query_string& out) {
        std::istringstream stream(qs);
        std::string pair;
        while (std::getline(stream, pair, '&')) {
            size_t eq = pair.find('=');
            if (eq != std::string::npos) {
                std::string k = url_decode(pair.substr(0, eq));
                std::string v = url_decode(pair.substr(eq + 1));
                out.params[k] = v;
            } else if (!pair.empty()) {
                out.params[url_decode(pair)] = "";
            }
        }
    }

    std::string url_decode(const std::string& in) {
        std::string out;
        for (size_t i = 0; i < in.length(); ++i) {
            if (in[i] == '%' && i + 2 < in.length()) {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    out += static_cast<char>(value);
                    i += 2;
                } else {
                    out += in[i];
                }
            } else if (in[i] == '+') {
                out += ' ';
            } else {
                out += in[i];
            }
        }
        return out;
    }

    void send_response(int client_fd, const response& res) {
        std::ostringstream ss;
        std::string status_msg = "OK";
        if (res.code == 201) status_msg = "Created";
        else if (res.code == 204) status_msg = "No Content";
        else if (res.code == 400) status_msg = "Bad Request";
        else if (res.code == 401) status_msg = "Unauthorized";
        else if (res.code == 403) status_msg = "Forbidden";
        else if (res.code == 404) status_msg = "Not Found";
        else if (res.code == 409) status_msg = "Conflict";
        else if (res.code == 500) status_msg = "Internal Server Error";

        ss << "HTTP/1.1 " << res.code << " " << status_msg << "\r\n";
        for (const auto& h : res.headers) {
            ss << h.first << ": " << h.second << "\r\n";
        }
        ss << "Content-Length: " << res.body.size() << "\r\n";
        ss << "Connection: close\r\n\r\n";
        ss << res.body;

        std::string out = ss.str();
        write(client_fd, out.data(), out.size());
    }
};

} // namespace crow

// Macro for standard Crow routing
#define CROW_ROUTE(app, url) app.route(url)

#endif // CROW_H
