#include "../include/entry.h"

namespace kissearch {
    term_info::term_info() {
        this->count = 0;
        this->score = 0;
    }

    field_number::field_number() {
        this->number = 0;
    }
    field_number::field_number(const size_t &number) {
        this->number = number;
    }
    field_number::field_number(const std::string &number) {
        this->number = std::stol(number);
    }

    bool field_number::operator==(const field_number &value) const {
        return this->number == value.number;
    }

    field_text::field_text() = default;
    field_text::field_text(const std::string &text) {
        this->text = text;
        this->terms.reserve(INIT_TERMS_SIZE);
        this->index.reserve(INIT_INDEX_SIZE);
    }

    std::string &field_text::find_term(const std::string &s) {
        auto found = std::find(terms.begin(), terms.end(), s);

        if (found == terms.end()) {
            terms.push_back(s);
            return terms.back();
        }

        return *found;
    }
    term_info &field_text::find_index(const std::string &s) {
        const auto lambada = [&](const index_t &c) { return c.first == s; };
        auto found = std::find_if(index.begin(), index.end(), lambada);

        if (found == index.end()) {
            index.emplace_back(s, term_info());
            return index.back().second;
        }

        return found->second;
    }

    __gnu_cxx::__normal_iterator<std::basic_string<char> *, std::vector<std::basic_string<char>>>
    field_text::find_term_it(const std::string &s) {
        return std::find(terms.begin(), terms.end(), s);
    }

    __gnu_cxx::__normal_iterator<field_text::index_t *, std::vector<field_text::index_t>>
    field_text::find_index_it(const std::string &s) {
        const auto lambada = [&](const index_t &c) { return c.first == s; };
        return std::find_if(index.begin(), index.end(), lambada);
    }

    bool field_text::operator==(const field_text &value) const {
        return this->text == value.text;
    }

    field_keyword::field_keyword() = default;
    field_keyword::field_keyword(const std::string &keyword) {
        this->keyword = keyword;
    }

    bool field_keyword::operator==(const field_keyword &value) const {
        return this->keyword == value.keyword;
    }

    entry::entry() {
        numbers.reserve(INIT_FIELDS_SIZE);
        texts.reserve(INIT_FIELDS_SIZE);
        keywords.reserve(INIT_FIELDS_SIZE);
    }

    field_number &entry::find_field_number(const std::string &s) {
        const auto lambada = [&](const field_number_t &c) { return c.first == s; };
        return std::find_if(numbers.begin(), numbers.end(), lambada)->second;
    }
    field_text &entry::find_field_text(const std::string &s) {
        const auto lambada = [&](const field_text_t &c) { return c.first == s; };
        return std::find_if(texts.begin(), texts.end(), lambada)->second;
    }
    field_keyword &entry::find_field_keyword(const std::string &s) {
        const auto lambada = [&](const field_keyword_t &c) { return c.first == s; };
        return std::find_if(keywords.begin(), keywords.end(), lambada)->second;
    }

    __gnu_cxx::__normal_iterator<entry::field_number_t *, std::vector<entry::field_number_t>>
    entry::find_field_number_it(const std::string &s) {
        const auto lambada = [&](const field_number_t &c) { return c.first == s; };
        return std::find_if(numbers.begin(), numbers.end(), lambada);
    }
    __gnu_cxx::__normal_iterator<entry::field_text_t *, std::vector<entry::field_text_t>>
    entry::find_field_text_it(const std::string &s) {
        const auto lambada = [&](const field_text_t &c) { return c.first == s; };
        return std::find_if(texts.begin(), texts.end(), lambada);
    }
    __gnu_cxx::__normal_iterator<entry::field_keyword_t *, std::vector<entry::field_keyword_t>>
    entry::find_field_keyword_it(const std::string &s) {
        const auto lambada = [&](const field_keyword_t &c) { return c.first == s; };
        return std::find_if(keywords.begin(), keywords.end(), lambada);
    }

    bool entry::operator ==(const entry &e) const {
        return this->numbers == e.numbers && this->texts == e.texts && this->keywords == e.keywords;
    }
}