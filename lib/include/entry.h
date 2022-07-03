#ifndef ENTRY_H
#define ENTRY_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <memory>

#define INIT_FIELDS_SIZE 4

namespace kissearch {
    struct field {
        struct number {
            ulong value;

            number();
            explicit number(const ulong &number);
            explicit number(const std::string &number);

            inline bool operator==(const number &v) const { return this->value == v.value; }
            inline bool operator==(const ulong &v) const { return this->value == v; }
            inline bool operator==(const std::string &v) const { return this->value == std::stol(v); }
        };

        struct text {
            std::string value;
            ulong terms_length;

            text();
            explicit text(const std::string &text);

            inline bool operator==(const text &v) const { return this->value == v.value; }
            inline bool operator==(const std::string &v) const { return this->value == v; }
        };

        struct keyword {
            std::string value;

            keyword();
            explicit keyword(const std::string &keyword);

            inline bool operator==(const keyword &v) const { return this->value == v.value; }
            inline bool operator==(const std::string &v) const { return this->value == v; }
        };

        struct boolean {
            bool value;

            boolean();
            explicit boolean(const bool &boolean);
            explicit boolean(const std::string &boolean);

            inline bool operator==(const boolean &v) const { return this->value == v.value; }
            inline bool operator==(const bool &v) const { return this->value == v; }
            inline bool operator==(const std::string &v) const { return this->value == (v == "true" || v == "1"); }
        };

        struct value {
            std::shared_ptr<number> _number;
            std::shared_ptr<text> _text;
            std::shared_ptr<keyword> _keyword;
            std::shared_ptr<boolean> _boolean;

            inline bool is_number() const { return _number != nullptr; }
            inline bool is_text() const { return _text != nullptr; }
            inline bool is_keyword() const { return _keyword != nullptr; }
            inline bool is_boolean() const { return _boolean != nullptr; }
        };

        std::string name;
        value val;

        std::string val_s();

        inline bool operator==(const field &f) const {
            return val._number == f.val._number
                   && val._text == f.val._text
                   && val._keyword == f.val._keyword
                   && val._boolean == f.val._boolean;
        }
    };

    struct entry {
        entry();

        std::vector<field> fields;

        inline field::value &find_field(const std::string &name) {
            const auto lambda = [&](const field &f) { return f.name == name; };
            return std::find_if(fields.begin(), fields.end(), lambda)->val;
        }

        inline bool operator==(const entry &e) const { return this->fields == e.fields; }
    };
}

#endif