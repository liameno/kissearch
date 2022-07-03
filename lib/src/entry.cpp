#include "../include/entry.h"

namespace kissearch {
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

        throw "val is undefined";
    }

    entry::entry() {
        fields.reserve(INIT_FIELDS_SIZE);
    }
}