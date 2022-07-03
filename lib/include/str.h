#ifndef STR_H
#define STR_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

namespace {
    std::vector<std::string> split(const std::string &s, const std::string &delimiter) {
        std::vector<std::string> result;
        size_t delimiter_length = delimiter.length();
        size_t start = 0;

        while (true) {
            size_t end = s.find(delimiter, start);

            if (end == std::string::npos) {
                break;
            }

            std::string token = s.substr(start, end - start);
            start = end + delimiter_length;
            result.push_back(token);
        }

        result.push_back(s.substr(start));
        return result;
    }
    size_t find_end(const std::string &s, const std::string &v) {
        auto s_length = s.length();
        auto v_length = v.length();

        if (v_length > s_length) return std::string::npos;
        return s.find(v, s_length - v_length);
    }
    bool replace(std::string &s, const std::string &from, const std::string &to = "") {
        if (from.length() > s.length()) return false;

        size_t start_pos = 0;
        bool result = false;

        while (true) {
            start_pos = s.find(from, start_pos);

            if (start_pos == std::string::npos) {
                break;
            }

            result = true;
            s.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }

        return result;
    }
    bool replace(std::string &s, size_t start_pos, const std::string &from, const std::string &to = "") {
        if (from.length() > s.length()) return false;

        bool result = false;

        while (true) {
            start_pos = s.find(from, start_pos);

            if (start_pos == std::string::npos) {
                break;
            }

            result = true;
            s.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }

        return result;
    }
    bool replace_end(std::string &s, const std::string &from, const std::string &to = "") {
        auto s_length = s.length();
        auto from_length = from.length();

        if (from_length > s_length) return false;
        return replace(s, s_length - from_length, from, to);
    }
    bool replace_end(std::string &s, const size_t &start, const std::string &from, const std::string &to = "") {
        if (from.length() > s.length()) return false;
        auto found = find_end(s, from);

        if (found != std::string::npos && found >= start) {
            return replace_end(s, from, to);
        }

        return false;
    }
    inline void trim_start(std::string &s) {
        static const auto lambda_space = [](const char &c) { return std::isspace(c); };
        auto first = std::find_if_not(s.begin(), s.end(), lambda_space);
        s.erase(s.begin(), first);
    }
    inline void trim_end(std::string &s) {
        static const auto lambda_space = [](const char &c) { return std::isspace(c); };
        auto last = std::find_if_not(s.rbegin(), s.rend(), lambda_space);
        s.erase(last.base(), s.end());
    }
    inline void to_lower(std::string &s) {
        for (auto &c : s) {
            c = std::tolower(c);
        }
    }
    inline void to_upper(std::string &s) {
        for (auto &c : s) {
            c = std::toupper(c);
        }
    }
    bool starts_with(const std::string &s, const std::string &v) {
        const size_t v_size = v.size();
        const size_t s_size = s.size();

        if (s_size < v_size) return false;

        for (int i = 0; i < v_size; ++i) {
            if (s[i] != v[i]) {
                return false;
            }
        }

        return true;
    }
    bool ends_with(const std::string &s, const std::string &v) {
        const size_t v_size = v.size();
        const size_t s_size = s.size();
        const size_t size = s_size - v_size;

        if (s_size < v_size) return false;

        for (int i = 0; i < v_size; ++i) {
            if (s[size + i] != v[i]) {
                return false;
            }
        }

        return true;
    }
    inline void remove_special_chars(std::string &s) {
        const auto lambda = [](const char &c) { return !std::isalpha(c) && !std::isdigit(c) && c != '\''; };
        s.erase(std::remove_if(s.begin(), s.end(), lambda), s.end());
    }
}

#endif