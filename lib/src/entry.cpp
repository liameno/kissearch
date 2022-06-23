#include "../include/entry.h"

namespace kissearch {
    term_info::term_info() {
        this->count = 0;
        this->score = 0;
    }

    field::number::number() {
        this->value = 0;
    }
    field::number::number(const ulong &value) {
        this->value = value;
    }
    field::number::number(const std::string &value) {
        this->value = std::stol(value);
    }

    field::text::text() = default;
    field::text::text(const std::string &value) {
        this->value = value;
        this->terms.reserve(INIT_TERMS_SIZE);
        this->index.reserve(INIT_INDEX_SIZE);
    }

    std::string &field::text::find_term(const std::string &s) {
        auto found = std::find(terms.begin(), terms.end(), s);

        if (found == terms.end()) {
            terms.push_back(s);
            return terms.back();
        }

        return *found;
    }
    term_info &field::text::find_index(const std::string &s) {
        const auto lambda = [&](const index_t &c) { return c.first == s; };
        auto found = std::find_if(index.begin(), index.end(), lambda);

        if (found == index.end()) {
            index.emplace_back(s, term_info());
            return index.back().second;
        }

        return found->second;
    }

    __gnu_cxx::__normal_iterator<std::basic_string<char> *, std::vector<std::basic_string<char>>>
    field::text::find_term_it(const std::string &s) {
        return std::find(terms.begin(), terms.end(), s);
    }

    __gnu_cxx::__normal_iterator<field::text::index_t *, std::vector<field::text::index_t>>
    field::text::find_index_it(const std::string &s) {
        const auto lambda = [&](const index_t &c) { return c.first == s; };
        return std::find_if(index.begin(), index.end(), lambda);
    }

    field::keyword::keyword() = default;
    field::keyword::keyword(const std::string &value) {
        this->value = value;
    }

    field::boolean::boolean() {
        this->value = false;
    }
    field::boolean::boolean(const bool &value) {
        this->value = value;
    }
    field::boolean::boolean(const std::string &value) {
        this->value = (value == "true" || value == "1");
    }

    std::string field::val_s() {
        if (val.is_number()) {
            return std::to_string(val._number->value);
        } else if (val.is_text()) {
            return val._text->value;
        } else if (val.is_keyword()) {
            return val._keyword->value;
        } else if (val.is_boolean()) {
            return std::to_string(val._boolean->value);
        }
    }

    entry::entry() {
        fields.reserve(INIT_FIELDS_SIZE);
    }
}