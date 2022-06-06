#include "../include/entry.h"

term_info::term_info() {
    count = 0;
    score = 0;
}

field_number::field_number() {
    number = 0;
}
field_number::field_number(const size_t &number) {
    this->number = number;
}
field_number::field_number(const std::string &number) {
    this->number = std::stol(number);
}

field_text::field_text() = default;
field_text::field_text(const std::string &text) {
    this->text = text;
}

field_keyword::field_keyword() = default;
field_keyword::field_keyword(const std::string &keyword) {
    this->keyword = keyword;
}