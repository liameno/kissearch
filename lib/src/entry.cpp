#include "../include/entry.h"

namespace kissearch {
    term_info::term_info() {
        this->count = 0;
        this->score = 0;
    }

    field::number::number() {
        this->value = 0;
    }
    field::number::number(const size_t &value) {
        this->value = value;
    }
    field::number::number(const std::string &value) {
        this->value = std::stol(value);
    }

    bool field::number::operator==(const number &v) const {
        return this->value == v.value;
    }
    bool field::number::operator==(const ulong &v) const {
        return this->value == v;
    }
    bool field::number::operator==(const std::string &v) const {
        return this->value == std::stol(v);
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

    bool field::text::operator==(const text &v) const {
        return this->value == v.value;
    }
    bool field::text::operator==(const std::string &v) const {
        return this->value == v;
    }

    field::keyword::keyword() = default;
    field::keyword::keyword(const std::string &value) {
        this->value = value;
    }

    bool field::keyword::operator==(const keyword &v) const {
        return this->value == v.value;
    }
    bool field::keyword::operator==(const std::string &v) const {
        return this->value == v;
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

    bool field::boolean::operator==(const boolean &v) const {
        return this->value == v.value;
    }
    bool field::boolean::operator==(const bool &v) const {
        return this->value == v;
    }
    bool field::boolean::operator==(const std::string &v) const {
        return this->value == (v == "true" || v == "1");
    }

    std::string field::val_s() {
        if (val.is_number()) {
            return std::to_string(val._number->value);
        }
        else if (val.is_text()) {
            return val._text->value;
        }
        else if (val.is_keyword()) {
            return val._keyword->value;
        }
        else if (val.is_boolean()) {
            return std::to_string(val._boolean->value);
        }
    }

    bool field::operator==(const field &f) const {
        return val._number == f.val._number
               && val._text == f.val._text
               && val._keyword == f.val._keyword
               && val._boolean == f.val._boolean;
    }

    entry::entry() {
        fields.reserve(INIT_FIELDS_SIZE);
        /*numbers.reserve(INIT_FIELDS_SIZE);
        texts.reserve(INIT_FIELDS_SIZE);
        keywords.reserve(INIT_FIELDS_SIZE);*/
    }

    field::value &entry::find_field(const std::string &name) {
        const auto lambda = [&](const field &f) { return f.name == name; };
        return std::find_if(fields.begin(), fields.end(), lambda)->val;
    }

    bool entry::operator ==(const entry &e) const {
        return this->fields == e.fields;
        //return this->numbers == e.numbers && this->values == e.texts && this->values == e.keywords;
    }
}