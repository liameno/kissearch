#include "../include/document.h"

#include "../include/lz4_frame_wrapper.h"
#include "../include/porter2.h"

std::string document::get_file_content(const std::string &file_name) {
        std::ifstream stream(file_name);
        std::string buffer;

        stream.seekg(0, std::ios::end);
        buffer.resize(stream.tellg());
        stream.seekg(0);
        stream.read(const_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }

void document::normalize(std::string &s) {
    to_lower(s);
    remove_special_chars(s);
}
std::vector<std::string> document::tokenize(std::string &text) {
    auto terms = split(text, " ");

    for (auto &s : terms) {
        normalize(s);
    }

    return terms;
}
void document::stem(std::vector<std::string> &terms) {
    static auto lambada = [](std::string &term) { return !porter2::english::stem(term); };
    terms.erase(std::remove_if(terms.begin(), terms.end(), lambada), terms.end());
}
void document::tokenize(entry &e, const std::string &field_name) {
    auto &field = e.texts[field_name];
    field.terms = tokenize(field.text);
}
void document::stem(entry &e, const std::string &field_name) {
    auto &field = e.texts[field_name];
    static auto lambada = [](std::string &term) { return !porter2::english::stem(term); };
    field.terms.erase(std::remove_if(field.terms.begin(), field.terms.end(), lambada), field.terms.end());
}
void document::sort_text_results(std::vector<result_t> &results) {
    auto results_size = results.size();

    for (int i = 0; i < results_size; i++) {
        bool swapped = false;

        for (int j = 0; j < results_size - i - 1; j++) {
            auto &c = results[j];
            auto &p1 = results[j + 1];

            if (p1.second > c.second) {
                std::swap(p1, c);
                swapped = true;
            }
        }

        if (!swapped) break;
    }
}

document::document(const double &k, const double &b) {
    this->k = k;
    this->b = b;
}

ulong document::compute_document_length_in_words(const std::string &field_name) {
    ulong size = 0;

    for (auto &entry : entries) {
        auto &field = entry.texts[field_name];
        size += field.terms.size();
    }

    return size;
}
double document::compute_tf(entry &e, const std::string &term, const std::string &field_name) {
    auto &field = e.texts[field_name];
    auto score = 0;

    for (auto &word: field.terms) {
        field.index[word].count = 0;
    }
    for (auto &w : field.terms) {
        if (term == w) {
            score++;
        }
    }

    return score;
}
double document::compute_idf(entry &e, const std::string &term, const std::string &field_name) {
    const auto size = entries.size();
    u_long found_size = 0;

    for (auto &entry2: entries) {
        auto &field = entry2.texts[field_name];

        if (std::find(field.terms.begin(), field.terms.end(), term) != field.terms.end()) {
            ++found_size;
        }
    }

    return std::log(((double)size - (double)found_size + 0.5) / ((double)found_size + 0.5) + 1);
}
double document::compute_bm25(entry &e, const std::string &term, const std::string &field_name) {
    auto document_length_in_words = (double)compute_document_length_in_words(field_name);
    auto entries_size = (double)entries.size();
    auto field_size = (double)e.texts[field_name].terms.size();

    auto tf = compute_tf(e, term, field_name);
    auto idf = compute_idf(e, term, field_name);
    auto avgdl = document_length_in_words / entries_size;

    return idf * (tf * (k + 1)) / (tf + k * (1 - b + b * field_size / avgdl));
}

void document::index_text_field(const std::string &field_name) {
    //tokenize, stem
    for (auto &entry : entries) {
        tokenize(entry, field_name);
        stem(entry, field_name);
    }
    //score
    for (auto &entry : entries) {
        auto &field = entry.texts[field_name];

        for (const auto &term : field.terms) {
            field.index[term].score = compute_bm25(entry, term, field_name);
        }
    }
}

std::vector<document::result_t> document::number_search(const u_long &query, const std::string &field_name) {
    std::vector<result_t> results;

    for (auto &entry : entries) {
        auto &field = entry.numbers[field_name];

        if (field.number == query) {
            results.emplace_back(entry, 1);
        }
    }

    return results;
}
std::vector<document::result_t> document::text_search(std::string query, const std::string &field_name) {
    auto terms = tokenize(query);
    stem(terms);

    std::vector<result_t> results;

    for (auto &entry : entries) {
        auto &field = entry.texts[field_name];
        double score = 0;

        for (auto &term : terms) {
            auto found = field.index.find(term);

            if (found != field.index.end()) {
                score += found->second.score;
            }
        }

        if (score <= 0) continue;
        results.emplace_back(entry, score);
    }

    return results;
}
std::vector<document::result_t> document::keyword_search(const std::string &query, const std::string &field_name) {
    std::vector<result_t> results;

    for (auto &entry : entries) {
        auto &field = entry.keywords[field_name];

        if (field.keyword == query) {
            results.emplace_back(entry, 1);
        }
    }

    return results;
}

void document::add(const entry &e) {
    this->entries.push_back(e);
}

void document::load(const std::string &file_name) {
    if (!entries.empty()) entries.clear();

    std::string tmp_file_name = file_name + ".tmp";

    FILE *input = fopen(file_name.c_str(), "rb");
    FILE *output = fopen(tmp_file_name.c_str(), "wb");

    lz4_frame_wrapper::decompress_file(input, output);

    fclose(input);
    fclose(output);

    auto decompressed = get_file_content(tmp_file_name);

    std::filesystem::remove(tmp_file_name);
    std::stringstream stream(decompressed);

    std::string s;
    getline(stream, s);

    entry e;

    std::string text_field;
    std::string index_field;

    while (getline(stream, s)) {
        if (s == ";") {
            add(e);
            continue;
        }

        auto start = 0;

        std::string key;
        std::string type;
        std::string value;

        while (true) {
            size_t end = s.find('/', start);

            if (end == std::string::npos) {
                break;
            }

            if (start == 0) {
                key = s.substr(start, end - start);
                start = end + 1;
            } else {
                type = s.substr(start, end - start);
                start = end + 1;
                break;
            }
        }

        const auto s_size = s.size();

        if (type.empty()) {
            type = key;
            key.clear();
        }

        value = s.substr(start, s_size - start);

        if (type == "n") {
            e.numbers[key] = field_number(value);
        }
        else if (type == "t") {
            e.texts[key] = field_text(value);
            text_field = key;
        }
        else if (type ==  "w") {
            e.texts[text_field].terms.push_back(value);
            index_field = value;
        }
        else if (type == "c") {
            e.texts[text_field].index[index_field].count = std::stol(value);
        }
        else if (type == "s") {
            e.texts[text_field].index[index_field].score = std::stof(value);
        }
        else if (type == "k") {
            e.keywords[key] = field_keyword(value);
        }
    }
}
void document::save(const std::string &file_name) {
    if (entries.empty()) return;
    if (std::filesystem::exists(file_name)) {
        std::filesystem::remove(file_name);
    }

    std::stringstream content;

    for (auto &entry : entries) {
        for (auto &number : entry.numbers) {
            content << number.first
                    << '/'
                    << "n"
                    << '/'
                    << number.second.number
                    << std::endl;
        }
        for (auto &text : entry.texts) {
            content << text.first
                    << '/'
                    << "t"
                    << '/'
                    << text.second.text
                    << std::endl;

            for (auto &term : text.second.terms) {
                content << "w"
                        << '/'
                        << term
                        << std::endl;
                content << "c"
                        << '/'
                        << text.second.index[term].count
                        << std::endl;
                content << "s"
                        << '/'
                        << text.second.index[term].score
                        << std::endl;
            }
        }
        for (auto &keyword : entry.keywords) {
            content << keyword.first
                    << '/'
                    << "k"
                    << '/'
                    << keyword.second.keyword
                    << std::endl;
        }

        content << ";" << std::endl;
    }

    std::string data = content.str();
    std::string tmp_file_name = file_name + ".tmp";

    std::ofstream file_tmp(tmp_file_name,std::ofstream::binary);
    file_tmp.write(data.data(), data.size());
    file_tmp.close();

    FILE *input = fopen(tmp_file_name.c_str(), "rb");
    FILE *output = fopen(file_name.c_str(), "wb");

    lz4_frame_wrapper::compress_file(input, output);

    fclose(input);
    fclose(output);

    std::filesystem::remove(tmp_file_name);
}
