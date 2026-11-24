// ============================================================================
// Lightweight nlohmann/json compatible single-header implementation
// For Student Result Management System (C++17)
// ============================================================================

#ifndef NLOHMANN_JSON_HPP
#define NLOHMANN_JSON_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cctype>
#include <initializer_list>

namespace nlohmann {

class json {
public:
    enum class Type { Null, Boolean, Number, String, Array, Object };

    Type type = Type::Null;
    bool bool_val = false;
    double num_val = 0.0;
    std::string str_val;
    std::vector<json> arr_val;
    std::map<std::string, json> obj_val;

    json() : type(Type::Null) {}
    json(std::nullptr_t) : type(Type::Null) {}
    json(bool b) : type(Type::Boolean), bool_val(b) {}
    json(int n) : type(Type::Number), num_val(static_cast<double>(n)) {}
    json(long n) : type(Type::Number), num_val(static_cast<double>(n)) {}
    json(long long n) : type(Type::Number), num_val(static_cast<double>(n)) {}
    json(unsigned int n) : type(Type::Number), num_val(static_cast<double>(n)) {}
    json(unsigned long n) : type(Type::Number), num_val(static_cast<double>(n)) {}
    json(double d) : type(Type::Number), num_val(d) {}
    json(float f) : type(Type::Number), num_val(static_cast<double>(f)) {}
    json(const char* s) : type(Type::String), str_val(s ? s : "") {}
    json(const std::string& s) : type(Type::String), str_val(s) {}

    // Support for {{"key", "val"}, {"k2", 123}} initializer lists
    json(std::initializer_list<std::pair<std::string, json>> init) : type(Type::Object) {
        for (const auto& kv : init) {
            obj_val[kv.first] = kv.second;
        }
    }

    static json array() {
        json j;
        j.type = Type::Array;
        return j;
    }

    static json object() {
        json j;
        j.type = Type::Object;
        return j;
    }

    bool is_null() const { return type == Type::Null; }
    bool is_boolean() const { return type == Type::Boolean; }
    bool is_number() const { return type == Type::Number; }
    bool is_string() const { return type == Type::String; }
    bool is_array() const { return type == Type::Array; }
    bool is_object() const { return type == Type::Object; }

    size_t size() const {
        if (type == Type::Array) return arr_val.size();
        if (type == Type::Object) return obj_val.size();
        return 0;
    }

    bool empty() const {
        if (type == Type::Array) return arr_val.empty();
        if (type == Type::Object) return obj_val.empty();
        return true;
    }

    bool contains(const std::string& key) const {
        if (type != Type::Object) return false;
        return obj_val.find(key) != obj_val.end();
    }

    json& operator[](const std::string& key) {
        if (type != Type::Object) {
            type = Type::Object;
            obj_val.clear();
        }
        return obj_val[key];
    }

    const json& operator[](const std::string& key) const {
        if (type != Type::Object) {
            static const json null_json;
            return null_json;
        }
        auto it = obj_val.find(key);
        if (it != obj_val.end()) return it->second;
        static const json null_json;
        return null_json;
    }

    json& operator[](size_t index) {
        if (type != Type::Array) {
            type = Type::Array;
            arr_val.clear();
        }
        if (index >= arr_val.size()) {
            arr_val.resize(index + 1);
        }
        return arr_val[index];
    }

    const json& operator[](size_t index) const {
        if (type != Type::Array || index >= arr_val.size()) {
            static const json null_json;
            return null_json;
        }
        return arr_val[index];
    }

    void push_back(const json& val) {
        if (type != Type::Array) {
            type = Type::Array;
            arr_val.clear();
        }
        arr_val.push_back(val);
    }

    template<typename T>
    T get() const;

    template<typename T>
    T value(const std::string& key, const T& default_value) const {
        if (type != Type::Object) return default_value;
        auto it = obj_val.find(key);
        if (it == obj_val.end() || it->second.is_null()) return default_value;
        try {
            return it->second.get<T>();
        } catch (...) {
            return default_value;
        }
    }

    std::string value(const std::string& key, const char* default_value) const {
        return value<std::string>(key, std::string(default_value ? default_value : ""));
    }

    // Iterators for array and object
    std::vector<json>::iterator begin() { return arr_val.begin(); }
    std::vector<json>::iterator end() { return arr_val.end(); }
    std::vector<json>::const_iterator begin() const { return arr_val.begin(); }
    std::vector<json>::const_iterator end() const { return arr_val.end(); }

    std::string dump(int indent = -1, int current_indent = 0) const {
        std::string ind = (indent > 0) ? std::string(current_indent, ' ') : "";
        std::string next_ind = (indent > 0) ? std::string(current_indent + indent, ' ') : "";
        std::string nl = (indent > 0) ? "\n" : "";
        std::string sp = (indent > 0) ? " " : "";

        switch (type) {
            case Type::Null: return "null";
            case Type::Boolean: return bool_val ? "true" : "false";
            case Type::Number: {
                std::ostringstream ss;
                if (num_val == static_cast<long long>(num_val)) {
                    ss << static_cast<long long>(num_val);
                } else {
                    ss << std::fixed << std::setprecision(2) << num_val;
                }
                return ss.str();
            }
            case Type::String: {
                std::string res = "\"";
                for (char c : str_val) {
                    if (c == '"') res += "\\\"";
                    else if (c == '\\') res += "\\\\";
                    else if (c == '\b') res += "\\b";
                    else if (c == '\f') res += "\\f";
                    else if (c == '\n') res += "\\n";
                    else if (c == '\r') res += "\\r";
                    else if (c == '\t') res += "\\t";
                    else res += c;
                }
                res += "\"";
                return res;
            }
            case Type::Array: {
                if (arr_val.empty()) return "[]";
                std::string res = "[" + nl;
                for (size_t i = 0; i < arr_val.size(); ++i) {
                    res += next_ind + arr_val[i].dump(indent, current_indent + indent);
                    if (i + 1 < arr_val.size()) res += ",";
                    res += nl;
                }
                res += ind + "]";
                return res;
            }
            case Type::Object: {
                if (obj_val.empty()) return "{}";
                std::string res = "{" + nl;
                size_t i = 0;
                for (const auto& kv : obj_val) {
                    res += next_ind + "\"" + kv.first + "\":" + sp + kv.second.dump(indent, current_indent + indent);
                    if (++i < obj_val.size()) res += ",";
                    res += nl;
                }
                res += ind + "}";
                return res;
            }
        }
        return "null";
    }

    // Simple, robust JSON parser
    static json parse(const std::string& str) {
        size_t idx = 0;
        return parse_val(str, idx);
    }

private:
    static void skip_ws(const std::string& s, size_t& i) {
        while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r')) {
            i++;
        }
    }

    static json parse_val(const std::string& s, size_t& i) {
        skip_ws(s, i);
        if (i >= s.size()) return json();

        char c = s[i];
        if (c == 'n') {
            i += 4; // null
            return json();
        }
        if (c == 't') {
            i += 4; // true
            return json(true);
        }
        if (c == 'f') {
            i += 5; // false
            return json(false);
        }
        if (c == '"') {
            return json(parse_str(s, i));
        }
        if (c == '[') {
            return parse_arr(s, i);
        }
        if (c == '{') {
            return parse_obj(s, i);
        }
        if (c == '-' || std::isdigit(c)) {
            return parse_num(s, i);
        }
        return json();
    }

    static std::string parse_str(const std::string& s, size_t& i) {
        i++; // skip opening '"'
        std::string res;
        while (i < s.size()) {
            char c = s[i++];
            if (c == '"') break;
            if (c == '\\' && i < s.size()) {
                char esc = s[i++];
                if (esc == '"') res += '"';
                else if (esc == '\\') res += '\\';
                else if (esc == '/') res += '/';
                else if (esc == 'b') res += '\b';
                else if (esc == 'f') res += '\f';
                else if (esc == 'n') res += '\n';
                else if (esc == 'r') res += '\r';
                else if (esc == 't') res += '\t';
                else res += esc;
            } else {
                res += c;
            }
        }
        return res;
    }

    static json parse_num(const std::string& s, size_t& i) {
        size_t start = i;
        if (s[i] == '-') i++;
        while (i < s.size() && (std::isdigit(s[i]) || s[i] == '.' || s[i] == 'e' || s[i] == 'E' || s[i] == '+' || s[i] == '-')) {
            i++;
        }
        try {
            double d = std::stod(s.substr(start, i - start));
            return json(d);
        } catch (...) {
            return json(0.0);
        }
    }

    static json parse_arr(const std::string& s, size_t& i) {
        i++; // skip '['
        json res = json::array();
        skip_ws(s, i);
        if (i < s.size() && s[i] == ']') {
            i++;
            return res;
        }
        while (i < s.size()) {
            res.push_back(parse_val(s, i));
            skip_ws(s, i);
            if (i < s.size() && s[i] == ',') {
                i++;
                skip_ws(s, i);
            } else if (i < s.size() && s[i] == ']') {
                i++;
                break;
            } else {
                break;
            }
        }
        return res;
    }

    static json parse_obj(const std::string& s, size_t& i) {
        i++; // skip '{'
        json res = json::object();
        skip_ws(s, i);
        if (i < s.size() && s[i] == '}') {
            i++;
            return res;
        }
        while (i < s.size()) {
            skip_ws(s, i);
            if (i >= s.size() || s[i] != '"') break;
            std::string key = parse_str(s, i);
            skip_ws(s, i);
            if (i < s.size() && s[i] == ':') {
                i++;
            }
            res[key] = parse_val(s, i);
            skip_ws(s, i);
            if (i < s.size() && s[i] == ',') {
                i++;
                skip_ws(s, i);
            } else if (i < s.size() && s[i] == '}') {
                i++;
                break;
            } else {
                break;
            }
        }
        return res;
    }
};

template<>
inline std::string json::get<std::string>() const {
    if (type == Type::String) return str_val;
    if (type == Type::Number) {
        std::ostringstream ss;
        ss << num_val;
        return ss.str();
    }
    if (type == Type::Boolean) return bool_val ? "true" : "false";
    return "";
}

template<>
inline double json::get<double>() const {
    if (type == Type::Number) return num_val;
    if (type == Type::String) {
        try { return std::stod(str_val); } catch (...) { return 0.0; }
    }
    return 0.0;
}

template<>
inline float json::get<float>() const {
    return static_cast<float>(get<double>());
}

template<>
inline int json::get<int>() const {
    return static_cast<int>(get<double>());
}

template<>
inline long json::get<long>() const {
    return static_cast<long>(get<double>());
}

template<>
inline long long json::get<long long>() const {
    return static_cast<long long>(get<double>());
}

template<>
inline bool json::get<bool>() const {
    if (type == Type::Boolean) return bool_val;
    if (type == Type::Number) return num_val != 0.0;
    if (type == Type::String) return str_val == "true" || str_val == "1";
    return false;
}

} // namespace nlohmann

#endif // NLOHMANN_JSON_HPP
