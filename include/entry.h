#ifndef ENTRY_H
#define ENTRY_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <memory>

namespace kissearch {
    struct term_info {
        ushort count;
        double score;

        term_info();
    };

    struct field_number {
        u_long number;

        field_number();
        explicit field_number(const size_t &number);
        explicit field_number(const std::string &number);
    };
    struct field_text {
        typedef std::pair<std::string, term_info> index_t;

        std::string text;
        std::vector<std::string> terms;
        std::vector<index_t> index;

        field_text();
        explicit field_text(const std::string &text);

        std::string &find_term(const std::string &s);
        term_info &find_index(const std::string &s);

        __gnu_cxx::__normal_iterator<std::basic_string<char> *, std::vector<std::basic_string<char>>>
        find_term_it(const std::string &s);
        __gnu_cxx::__normal_iterator<field_text::index_t *, std::vector<field_text::index_t>>
        find_index_it(const std::string &s);
    };
    struct field_keyword {
        std::string keyword;

        field_keyword();
        explicit field_keyword(const std::string &keyword);
    };

    struct entry {
        typedef std::pair<std::string, field_number> field_number_t;
        typedef std::pair<std::string, field_text> field_text_t;
        typedef std::pair<std::string, field_keyword> field_keyword_t;

        entry();

        std::vector<field_number_t> numbers;
        std::vector<field_text_t> texts;
        std::vector<field_keyword_t> keywords;

        field_number &find_field_number(const std::string &s);
        field_text &find_field_text(const std::string &s);
        field_keyword &find_field_keyword(const std::string &s);

        __gnu_cxx::__normal_iterator<entry::field_number_t *, std::vector<entry::field_number_t>>
        find_field_number_it(const std::string &s);
        __gnu_cxx::__normal_iterator<entry::field_text_t *, std::vector<entry::field_text_t>>
        find_field_text_it(const std::string &s);
        __gnu_cxx::__normal_iterator<entry::field_keyword_t *, std::vector<entry::field_keyword_t>>
        find_field_keyword_it(const std::string &s);
    };
}

#endif