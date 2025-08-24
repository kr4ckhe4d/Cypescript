// Cypescript Standard Library - C++ Integration
// This provides essential functions that Cypescript can call

#include <cmath>
#include <string>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <sstream>
#include <vector>
#include <map>
#include <cctype>

extern "C" {
    // ===================
    // MATH FUNCTIONS
    // ===================
    
    double math_sqrt(double x) {
        return std::sqrt(x);
    }
    
    double math_pow(double base, double exp) {
        return std::pow(base, exp);
    }
    
    double math_abs_f64(double x) {
        return std::abs(x);
    }
    
    int math_abs_i32(int x) {
        return std::abs(x);
    }
    
    double math_sin(double x) {
        return std::sin(x);
    }
    
    double math_cos(double x) {
        return std::cos(x);
    }
    
    double math_tan(double x) {
        return std::tan(x);
    }
    
    double math_log(double x) {
        return std::log(x);
    }
    
    double math_exp(double x) {
        return std::exp(x);
    }
    
    // ===================
    // STRING FUNCTIONS
    // ===================
    
    const char* string_reverse(const char* str) {
        if (!str) return nullptr;
        std::string s(str);
        std::reverse(s.begin(), s.end());
        char* result = new char[s.length() + 1];
        strcpy(result, s.c_str());
        return result;
    }
    
    const char* string_upper(const char* str) {
        if (!str) return nullptr;
        std::string s(str);
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        char* result = new char[s.length() + 1];
        strcpy(result, s.c_str());
        return result;
    }
    
    const char* string_lower(const char* str) {
        if (!str) return nullptr;
        std::string s(str);
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        char* result = new char[s.length() + 1];
        strcpy(result, s.c_str());
        return result;
    }
    
    int string_length(const char* str) {
        return str ? strlen(str) : 0;
    }
    
    const char* string_substring(const char* str, int start, int length) {
        if (!str) return nullptr;
        std::string s(str);
        if (start < 0 || start >= (int)s.length()) return nullptr;
        
        std::string sub = s.substr(start, length);
        char* result = new char[sub.length() + 1];
        strcpy(result, sub.c_str());
        return result;
    }
    
    int string_find(const char* str, const char* substr) {
        if (!str || !substr) return -1;
        std::string s(str);
        std::string sub(substr);
        size_t pos = s.find(sub);
        return pos == std::string::npos ? -1 : (int)pos;
    }
    
    const char* string_concat(const char* str1, const char* str2) {
        if (!str1 && !str2) return nullptr;
        if (!str1) str1 = "";
        if (!str2) str2 = "";
        
        std::string result = std::string(str1) + std::string(str2);
        char* cstr = new char[result.length() + 1];
        strcpy(cstr, result.c_str());
        return cstr;
    }
    
    // ===================
    // ARRAY FUNCTIONS
    // ===================
    
    int array_sum_i32(int* arr, int size) {
        if (!arr || size <= 0) return 0;
        int sum = 0;
        for (int i = 0; i < size; i++) {
            sum += arr[i];
        }
        return sum;
    }
    
    int array_max_i32(int* arr, int size) {
        if (!arr || size <= 0) return 0;
        int max = arr[0];
        for (int i = 1; i < size; i++) {
            if (arr[i] > max) max = arr[i];
        }
        return max;
    }
    
    int array_min_i32(int* arr, int size) {
        if (!arr || size <= 0) return 0;
        int min = arr[0];
        for (int i = 1; i < size; i++) {
            if (arr[i] < min) min = arr[i];
        }
        return min;
    }
    
    void array_sort_i32(int* arr, int size) {
        if (!arr || size <= 0) return;
        std::sort(arr, arr + size);
    }
    
    void array_reverse_i32(int* arr, int size) {
        if (!arr || size <= 0) return;
        std::reverse(arr, arr + size);
    }
    
    // ===================
    // FILE I/O FUNCTIONS
    // ===================
    
    const char* file_read(const char* filename) {
        if (!filename) return nullptr;
        
        std::ifstream file(filename);
        if (!file.is_open()) return nullptr;
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        char* result = new char[content.length() + 1];
        strcpy(result, content.c_str());
        return result;
    }
    
    int file_write(const char* filename, const char* content) {
        if (!filename || !content) return 0;
        
        std::ofstream file(filename);
        if (!file.is_open()) return 0;
        
        file << content;
        file.close();
        return 1; // success
    }
    
    int file_exists(const char* filename) {
        if (!filename) return 0;
        std::ifstream file(filename);
        return file.good() ? 1 : 0;
    }
    
    // ===================
    // UTILITY FUNCTIONS
    // ===================
    
    void sleep_ms(int milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
    
    int random_int(int min, int max) {
        if (min > max) std::swap(min, max);
        return min + (rand() % (max - min + 1));
    }
    
    double random_double() {
        return (double)rand() / RAND_MAX;
    }
    
    void random_seed(int seed) {
        srand(seed);
    }
    
    // ===================
    // JSON FUNCTIONS
    // ===================
    
    // Helper function to escape JSON strings
    std::string escape_json_string(const std::string& str) {
        std::string result = "\"";
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default:
                    if (c < 32) {
                        result += "\\u";
                        char hex[5];
                        sprintf(hex, "%04x", (unsigned char)c);
                        result += hex;
                    } else {
                        result += c;
                    }
                    break;
            }
        }
        result += "\"";
        return result;
    }
    
    // Helper function to unescape JSON strings
    std::string unescape_json_string(const std::string& str) {
        std::string result;
        for (size_t i = 1; i < str.length() - 1; ++i) { // Skip quotes
            if (str[i] == '\\' && i + 1 < str.length() - 1) {
                switch (str[i + 1]) {
                    case '"': result += '"'; i++; break;
                    case '\\': result += '\\'; i++; break;
                    case 'b': result += '\b'; i++; break;
                    case 'f': result += '\f'; i++; break;
                    case 'n': result += '\n'; i++; break;
                    case 'r': result += '\r'; i++; break;
                    case 't': result += '\t'; i++; break;
                    case 'u':
                        if (i + 5 < str.length()) {
                            std::string hex = str.substr(i + 2, 4);
                            int code = std::stoi(hex, nullptr, 16);
                            result += (char)code;
                            i += 5;
                        }
                        break;
                    default:
                        result += str[i + 1];
                        i++;
                        break;
                }
            } else {
                result += str[i];
            }
        }
        return result;
    }
    
    // Skip whitespace in JSON string
    size_t skip_whitespace(const std::string& json, size_t pos) {
        while (pos < json.length() && std::isspace(json[pos])) {
            pos++;
        }
        return pos;
    }
    
    // Parse JSON string value
    std::pair<std::string, size_t> parse_json_string(const std::string& json, size_t pos) {
        if (pos >= json.length() || json[pos] != '"') {
            return {"", pos};
        }
        
        size_t start = pos;
        pos++; // Skip opening quote
        
        while (pos < json.length() && json[pos] != '"') {
            if (json[pos] == '\\') {
                pos += 2; // Skip escaped character
            } else {
                pos++;
            }
        }
        
        if (pos < json.length()) {
            pos++; // Skip closing quote
        }
        
        return {json.substr(start, pos - start), pos};
    }
    
    // Parse JSON number value
    std::pair<std::string, size_t> parse_json_number(const std::string& json, size_t pos) {
        size_t start = pos;
        
        if (pos < json.length() && json[pos] == '-') {
            pos++;
        }
        
        while (pos < json.length() && std::isdigit(json[pos])) {
            pos++;
        }
        
        if (pos < json.length() && json[pos] == '.') {
            pos++;
            while (pos < json.length() && std::isdigit(json[pos])) {
                pos++;
            }
        }
        
        if (pos < json.length() && (json[pos] == 'e' || json[pos] == 'E')) {
            pos++;
            if (pos < json.length() && (json[pos] == '+' || json[pos] == '-')) {
                pos++;
            }
            while (pos < json.length() && std::isdigit(json[pos])) {
                pos++;
            }
        }
        
        return {json.substr(start, pos - start), pos};
    }
    
    // Create a simple JSON object string
    const char* json_create_object() {
        std::string* result = new std::string("{}");
        return result->c_str();
    }
    
    // Create a simple JSON array string
    const char* json_create_array() {
        std::string* result = new std::string("[]");
        return result->c_str();
    }
    
    // Add string property to JSON object
    const char* json_add_string(const char* json_obj, const char* key, const char* value) {
        std::string json_str(json_obj);
        std::string key_str(key);
        std::string value_str(value);
        
        // Remove closing brace
        if (json_str.back() == '}') {
            json_str.pop_back();
        }
        
        // Add comma if not empty
        if (json_str.length() > 1) {
            json_str += ",";
        }
        
        // Add key-value pair
        json_str += escape_json_string(key_str) + ":" + escape_json_string(value_str) + "}";
        
        std::string* result = new std::string(json_str);
        return result->c_str();
    }
    
    // Add number property to JSON object
    const char* json_add_number(const char* json_obj, const char* key, double value) {
        std::string json_str(json_obj);
        std::string key_str(key);
        
        // Remove closing brace
        if (json_str.back() == '}') {
            json_str.pop_back();
        }
        
        // Add comma if not empty
        if (json_str.length() > 1) {
            json_str += ",";
        }
        
        // Add key-value pair
        std::ostringstream oss;
        oss << value;
        json_str += escape_json_string(key_str) + ":" + oss.str() + "}";
        
        std::string* result = new std::string(json_str);
        return result->c_str();
    }
    
    // Add integer property to JSON object (convenience function)
    const char* json_add_int(const char* json_obj, const char* key, int value) {
        return json_add_number(json_obj, key, (double)value);
    }
    
    // Add boolean property to JSON object
    const char* json_add_boolean(const char* json_obj, const char* key, int value) {
        std::string json_str(json_obj);
        std::string key_str(key);
        
        // Remove closing brace
        if (json_str.back() == '}') {
            json_str.pop_back();
        }
        
        // Add comma if not empty
        if (json_str.length() > 1) {
            json_str += ",";
        }
        
        // Add key-value pair
        json_str += escape_json_string(key_str) + ":" + (value ? "true" : "false") + "}";
        
        std::string* result = new std::string(json_str);
        return result->c_str();
    }
    
    // Get string value from JSON object
    const char* json_get_string(const char* json_obj, const char* key) {
        std::string json_str(json_obj);
        std::string key_str = "\"" + std::string(key) + "\""; // Just add quotes, don't escape
        
        size_t key_pos = json_str.find(key_str);
        if (key_pos == std::string::npos) {
            return nullptr;
        }
        
        size_t colon_pos = json_str.find(':', key_pos);
        if (colon_pos == std::string::npos) {
            return nullptr;
        }
        
        size_t value_start = skip_whitespace(json_str, colon_pos + 1);
        if (value_start >= json_str.length() || json_str[value_start] != '"') {
            return nullptr;
        }
        
        auto [value_str, end_pos] = parse_json_string(json_str, value_start);
        if (value_str.empty()) {
            return nullptr;
        }
        
        std::string unescaped = unescape_json_string(value_str);
        
        std::string* result = new std::string(unescaped);
        return result->c_str();
    }
    
    // Get number value from JSON object
    double json_get_number(const char* json_obj, const char* key) {
        std::string json_str(json_obj);
        std::string key_str = "\"" + std::string(key) + "\""; // Just add quotes
        
        size_t key_pos = json_str.find(key_str);
        if (key_pos == std::string::npos) {
            return 0.0;
        }
        
        size_t colon_pos = json_str.find(':', key_pos);
        if (colon_pos == std::string::npos) {
            return 0.0;
        }
        
        // Skip whitespace after colon
        size_t value_start = colon_pos + 1;
        while (value_start < json_str.length() && std::isspace(json_str[value_start])) {
            value_start++;
        }
        
        if (value_start >= json_str.length()) {
            return 0.0;
        }
        
        // Find the end of the number (comma, brace, or end of string)
        size_t value_end = value_start;
        while (value_end < json_str.length() && 
               json_str[value_end] != ',' && 
               json_str[value_end] != '}' && 
               json_str[value_end] != ']' &&
               !std::isspace(json_str[value_end])) {
            value_end++;
        }
        
        if (value_end <= value_start) {
            return 0.0;
        }
        
        std::string number_str = json_str.substr(value_start, value_end - value_start);
        
        try {
            return std::stod(number_str);
        } catch (const std::exception&) {
            return 0.0;
        }
    }
    
    // Get integer value from JSON object (convenience function)
    int json_get_int(const char* json_obj, const char* key) {
        return (int)json_get_number(json_obj, key);
    }
    
    // Get boolean value from JSON object
    int json_get_boolean(const char* json_obj, const char* key) {
        std::string json_str(json_obj);
        std::string key_str = "\"" + std::string(key) + "\""; // Just add quotes, don't escape
        
        size_t key_pos = json_str.find(key_str);
        if (key_pos == std::string::npos) {
            return 0;
        }
        
        size_t colon_pos = json_str.find(':', key_pos);
        if (colon_pos == std::string::npos) {
            return 0;
        }
        
        size_t value_start = skip_whitespace(json_str, colon_pos + 1);
        
        if (json_str.substr(value_start, 4) == "true") {
            return 1;
        } else if (json_str.substr(value_start, 5) == "false") {
            return 0;
        }
        
        return 0;
    }
    
    // Parse JSON from string and validate
    int json_is_valid(const char* json_str) {
        std::string json(json_str);
        size_t pos = skip_whitespace(json, 0);
        
        if (pos >= json.length()) {
            return 0;
        }
        
        char first_char = json[pos];
        if (first_char != '{' && first_char != '[' && first_char != '"' && 
            !std::isdigit(first_char) && first_char != '-' && 
            json.substr(pos, 4) != "true" && json.substr(pos, 5) != "false" && 
            json.substr(pos, 4) != "null") {
            return 0;
        }
        
        return 1; // Basic validation - could be more comprehensive
    }
    
    // Convert JSON to pretty-printed format
    const char* json_prettify(const char* json_str) {
        std::string json(json_str);
        std::string result;
        int indent_level = 0;
        bool in_string = false;
        bool escaped = false;
        
        for (size_t i = 0; i < json.length(); ++i) {
            char c = json[i];
            
            if (!in_string && std::isspace(c)) {
                continue; // Skip whitespace outside strings
            }
            
            if (c == '"' && !escaped) {
                in_string = !in_string;
            }
            
            if (!in_string) {
                if (c == '{' || c == '[') {
                    result += c;
                    result += '\n';
                    indent_level++;
                    for (int j = 0; j < indent_level * 2; ++j) {
                        result += ' ';
                    }
                } else if (c == '}' || c == ']') {
                    result += '\n';
                    indent_level--;
                    for (int j = 0; j < indent_level * 2; ++j) {
                        result += ' ';
                    }
                    result += c;
                } else if (c == ',') {
                    result += c;
                    result += '\n';
                    for (int j = 0; j < indent_level * 2; ++j) {
                        result += ' ';
                    }
                } else if (c == ':') {
                    result += c;
                    result += ' ';
                } else {
                    result += c;
                }
            } else {
                result += c;
            }
            
            escaped = (c == '\\' && !escaped);
        }
        
        std::string* final_result = new std::string(result);
        return final_result->c_str();
    }
    
    // Minify JSON (remove unnecessary whitespace)
    const char* json_minify(const char* json_str) {
        std::string json(json_str);
        std::string result;
        bool in_string = false;
        bool escaped = false;
        
        for (char c : json) {
            if (c == '"' && !escaped) {
                in_string = !in_string;
            }
            
            if (in_string || !std::isspace(c)) {
                result += c;
            }
            
            escaped = (c == '\\' && !escaped);
        }
        
        std::string* final_result = new std::string(result);
        return final_result->c_str();
    }
    
    // Memory management helper
    void free_string(const char* str) {
        delete[] str;
    }
}
