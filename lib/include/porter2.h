#ifndef PORTER2_H
#define PORTER2_H

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>
#include <chrono>
#include <array>
#include <utility>

#include "str.h"

namespace porter2 {
    namespace english {
        bool contains_vowel(const std::string &s, const int &start, const int &end);
        bool if_contains_vowel_replace_end(std::string &s, const std::string &from, const std::string &to = "");
        bool if_contains_vowel_replace_end(std::string &s, const int &end, const std::string &from, const std::string &to = "");
        bool ends_with_double(const std::string &s);
        int first_non_vowel(const std::string &s, const int &start = 1);

        bool is_double_suffix(const std::string &s);
        bool is_vowel(const char &c);
        bool is_vowel_without_y(const char &c);
        bool is_valid_li_ending(const char &c);
        bool is_short(const std::string &s);

        void set_initial_y(std::string &s, const int &start = 1);

        bool is_special(std::string &s);
        bool is_special_1a(const std::string &s);
        int is_special_r1(const std::string &word);

        bool is_stop(const std::string &s);

        void step_0(std::string &word);

        void step_1a(std::string &word);
        void step_1b(std::string &word, const int &r1);
        void step_1c(std::string &word);

        void step_2(std::string &word, const int &r1);

        void step_3(std::string &word, const int &r1, const int &r2);

        void step_4(std::string &word, const int &r2);

        void step_5(std::string &word, const int &r1, const int &r2);

        bool stem(std::string &word);
    }
}

#endif