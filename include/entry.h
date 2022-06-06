#ifndef ENTRY_H
#define ENTRY_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>

struct term_info {
    u_short count;
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
    std::string text;
    std::vector<std::string> terms;
    std::unordered_map<std::string, term_info> index;

    field_text();
    explicit field_text(const std::string &text);
};
struct field_keyword {
    std::string keyword;

    field_keyword();
    explicit field_keyword(const std::string &keyword);
};

struct entry {
    std::unordered_map<std::string, field_number> numbers;
    std::unordered_map<std::string, field_text> texts;
    std::unordered_map<std::string, field_keyword> keywords;
};

#endif