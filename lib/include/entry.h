#ifndef ENTRY_H
#define ENTRY_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <memory>

#define INIT_TERMS_SIZE 30
#define INIT_INDEX_SIZE 20
#define INIT_FIELDS_SIZE 4

namespace kissearch {
    struct term_info {
        ushort count;
        double score;

        term_info();
    };

    struct field {
        struct number {
            ulong value;

            number();
            explicit number(const size_t &number);
            explicit number(const std::string &number);

            bool operator==(const number &v) const;
            bool operator==(const ulong &v) const;
            bool operator==(const std::string &v) const;
        };
        struct text {
            typedef std::pair<std::string, term_info> index_t;

            std::string value;
            std::vector<std::string> terms;
            std::vector<index_t> index;

            text();
            explicit text(const std::string &text);

            std::string &find_term(const std::string &s);
            term_info &find_index(const std::string &s);

            __gnu_cxx::__normal_iterator<std::basic_string<char> *, std::vector<std::basic_string<char>>>
            find_term_it(const std::string &s);
            __gnu_cxx::__normal_iterator<index_t *, std::vector<index_t>>
            find_index_it(const std::string &s);

            bool operator==(const text &v) const;
            bool operator==(const std::string &v) const;
        };
        struct keyword {
            std::string value;

            keyword();
            explicit keyword(const std::string &keyword);

            bool operator==(const keyword &v) const;
            bool operator==(const std::string &v) const;
        };
        struct boolean {
            bool value;

            boolean();
            explicit boolean(const bool &boolean);
            explicit boolean(const std::string &boolean);

            bool operator==(const boolean &v) const;
            bool operator==(const bool &v) const;
            bool operator==(const std::string &v) const;
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

        bool operator==(const field &f) const;
    };

    struct entry {
        entry();

        std::vector<field> fields;

        field::value &find_field(const std::string &name);

        bool operator ==(const entry &e) const;
    };
}

#endif