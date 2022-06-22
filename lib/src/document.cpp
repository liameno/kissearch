#include "../include/document.h"

#include "../include/porter2.h"
#include "../include/compression.h"

namespace kissearch {
    document::search_options::search_options() {
        this->sort_by_score = true;
        this->page = 1;
        this->page_size = 10;
    }

    std::string document::get_file_content(const std::string &file_name) {
            std::ifstream stream(file_name);
            std::string buffer;

            stream.seekg(0, std::ios::end);
            buffer.resize(stream.tellg());
            stream.seekg(0);
            stream.read(const_cast<char *>(buffer.data()), (int)buffer.size());

            return buffer;
        }

    void document::normalize(std::string &s) {
        to_lower(s);
        remove_special_chars(s);
    }
    std::vector<std::string> document::tokenize(const std::string &text) {
        auto terms = split(text, " ");

        for (auto &s : terms) {
            normalize(s);
        }

        return terms;
    }
    void document::stem(std::vector<std::string> &terms) {
        static auto lambda = [](std::string &term) { return !porter2::english::stem(term); };
        terms.erase(std::remove_if(terms.begin(), terms.end(), lambda), terms.end());
    }
    void document::tokenize(field::value &field) {
        field._text->terms = tokenize(field._text->value);
    }
    void document::stem(field::value &field) {
        static auto lambda = [](std::string &term) { return !porter2::english::stem(term); };
        field._text->terms.erase(std::remove_if(field._text->terms.begin(), field._text->terms.end(), lambda), field._text->terms.end());
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

    void document::write_block(std::stringstream &content, const std::string &type, const std::string &value) {
        content << type
                << '/'
                << value
                << std::endl;
    }
    void document::write_block(std::stringstream &content, const std::string &key, const std::string &type, const std::string &value) {
        content << key
                << '/'
                << type
                << '/'
                << value
                << std::endl;
    }
    void document::parse_block(const std::string &s, std::string &key, std::string &type, std::string &value) {
        int start = 0;

        while (true) {
            size_t end = s.find('/', start);

            if (end == std::string::npos) {
                break;
            }

            if (start == 0) {
                key = s.substr(start, end - start);
                start = (int)end + 1;
            } else {
                type = s.substr(start, end - start);
                start = (int)end + 1;
                break;
            }
        }

        if (type.empty()) {
            type = key;
            key.clear();
        }

        value = s.substr(start, s.size() - start);
    }

    document::document(const ulong &cache_idf_size, const double &k, const double &b) {
        this->cache_idf_size = cache_idf_size;
        this->k = k;
        this->b = b;
        clear_cache_idf();
    }

    ulong document::compute_document_length_in_words(const std::string &field_name) {
        ulong size = 0;

        for (auto &entry : entries) {
            auto &field = entry.find_field(field_name);
            size += field._text->terms.size();
        }

        return size;
    }
    double document::compute_tf(entry &e, const std::string &term, field::value &field) {
        auto score = 0;

        for (auto &word: field._text->terms) {
            field._text->find_index(word).count = 0;
        }
        for (auto &w : field._text->terms) {
            if (term == w) {
                score++;
            }
        }

        return score;
    }
    double document::compute_idf(entry &e, const std::string &term, const std::string &field_name) {
        const static auto lambda = [&](const std::pair<std::string, double> &c) { return c.first == term; };
        auto found_cache = std::find_if(cache_idf.begin(), cache_idf.end(), lambda);
        if (found_cache != cache_idf.end()) return found_cache->second;

        const auto size = entries.size();
        ulong found_size = 0;

        for (auto &entry2: entries) {
            auto &field = entry2.find_field(field_name);

            if (field._text->find_term_it(term) != field._text->terms.end()) {
                ++found_size;
            }
        }

        auto idf = std::log(((double)size - (double)found_size + 0.5) / ((double)found_size + 0.5) + 1);
        cache_idf.emplace_back(term, idf);
        return idf;
    }
    double document::compute_bm25(entry &e, const std::string &term, field::value &field, const std::string &field_name, const ulong &document_length_in_words) {
        auto entries_size = (double)entries.size();
        auto field_size = (double)field._text->terms.size();

        auto tf = compute_tf(e, term, field);
        auto idf = compute_idf(e, term, field_name);
        auto avgdl = (double)document_length_in_words / entries_size;

        return idf * (tf * (k + 1)) / (tf + k * (1 - b + b * field_size / avgdl));
    }

    ulong document::compute_next_number_value(const std::string &field_name) {
        if (entries.empty()) return 1;
        return entries.back().find_field(field_name)._number->value + 1;
    }

    void document::clear_cache_idf() {
        cache_idf = std::vector<cache_idf_t>(cache_idf_size);
    }
    void document::index_text_field(const std::string &field_name) {
        mutex.lock();
        clear_cache_idf();

        //tokenize, stem
        for (auto &entry : entries) {
            auto &field = entry.find_field(field_name);

            tokenize(field);
            stem(field);
        }

        const auto document_length_in_words = compute_document_length_in_words(field_name);

        //score
        for (auto &entry : entries) {
            auto &field = entry.find_field(field_name);

            for (const auto &term : field._text->terms) {
                field._text->find_index(term).score = compute_bm25(entry, term, field, field_name, document_length_in_words);
            }
        }
        mutex.unlock();
    }

    void document::slice_page(std::vector<result_t> &results, const search_options &options) {
        const auto page_size_max = options.page * options.page_size;
        const auto page_size_min = page_size_max - options.page_size;

        std::vector<result_t> results_tmp;
        results_tmp.reserve(options.page_size);

        for (int i = 0; i < results.size(); ++i) {
            if (i >= page_size_min && i < page_size_max) {
                results_tmp.push_back(results[i]);
            }
        }

        results = results_tmp;
    }

    std::vector<document::result_t> document::search(const std::string &query, const search_options &options, const bool is_delete) {
        const auto page_size_max = options.page * options.page_size;

        auto terms = tokenize(query);
        stem(terms);

        std::vector<result_t> results;

        for (const auto &field_name : options.field_names) {
            for (auto &entry: entries) {
                if (!is_delete && results.size() >= page_size_max) break;

                auto &field = entry.find_field(field_name);
                double score = 0;

                if (field.is_number()) {
                    if (field._number->operator==(std::stol(query))) {
                        score = 1;
                    }
                }
                else if (field.is_text()) {
                    for (auto &term: terms) {
                        auto found = field._text->find_index_it(term);

                        if (found != field._text->index.end()) {
                            score += found->second.score;
                        }
                    }
                }
                else if (field.is_keyword()) {
                    if (field._keyword->operator==(query)) {
                        score = 1;
                    }
                }
                else if (field.is_boolean()) {
                    if (field._boolean->operator==(query)) {
                        score = 1;
                    }
                }

                if (score <= 0) continue;

                const auto lambda = [&](const result_t &c) { return c.first == entry; };
                auto found = std::find_if(results.begin(), results.end(), lambda);

                if (found != results.end()) found->second += score;
                else results.emplace_back(entry, score);
            }
        }

        if (!is_delete) slice_page(results, options);
        if (options.sort_by_score) sort_text_results(results);

        return results;
    }

    void document::add(const entry &e) {
        mutex.lock();
        this->entries.push_back(e);
        mutex.unlock();
    }

    void document::load(const std::string &file_name) {
        if (!entries.empty()) {
            entries.clear();
        }

        auto decompressed = compression::decompress(get_file_content(file_name));
        std::stringstream stream(decompressed);

        std::string s;
        entry e;

        std::string key;
        std::string type;
        std::string value;

        getline(stream, s);
        parse_block(s, key, type, value);

        auto t = type.front();
        if (t == 'd') name = value;

        std::string text_field;
        std::string index_field;

        while (getline(stream, s)) {
            if (s == ";") {
                add(e);
                e = entry();
                continue;
            }

            key.clear();
            type.clear();
            value.clear();
            parse_block(s, key, type, value);
            t = type.front();

            if (t == 'n') {
                field f {key};
                f.val._number = std::make_shared<field::number>(value);
                e.fields.push_back(f);
            }
            else if (t == 't') {
                field f {key};
                f.val._text = std::make_shared<field::text>(value);
                e.fields.push_back(f);
                text_field = key;
            }
            else if (t == 'w') {
                e.find_field(text_field)._text->terms.push_back(value);
                index_field = value;
            }
            else if (t == 'c') {
                e.find_field(text_field)._text->find_index(index_field).count = std::stol(value);
            }
            else if (t == 's') {
                e.find_field(text_field)._text->find_index(index_field).score = std::stod(value);
            }
            else if (t == 'k') {
                field f {key};
                f.val._keyword = std::make_shared<field::keyword>(value);
                e.fields.push_back(f);
            }
            else if (t == 'b') {
                field f {key};
                f.val._boolean = std::make_shared<field::boolean>(value);
                e.fields.push_back(f);
            }
        }
    }
    void document::save(const std::string &file_name) {
        if (std::filesystem::exists(file_name)) {
            std::filesystem::remove(file_name);
        }

        std::stringstream content;
        write_block(content,  "d", name);

        for (auto &entry : entries) {
            for (auto &f : entry.fields) {
                std::string type;
                std::string value;

                if (f.val.is_number()) {
                    value = std::to_string(f.val._number->value);
                    type = "n";
                }
                else if (f.val.is_text()) {
                    value = f.val._text->value;
                    type = "t";
                }
                else if (f.val.is_keyword()) {
                    value = f.val._keyword->value;
                    type = "k";
                }
                else if (f.val.is_boolean()) {
                    value = std::to_string(f.val._boolean->value);
                    type = "b";
                }

                write_block(content, f.name, type, value);

                if (type == "t") {
                    for (auto &term : f.val._text->terms) {
                        auto &index = f.val._text->find_index(term);
                        write_block(content, "w", term);
                        write_block(content, "c", std::to_string(index.count));
                        write_block(content, "s", std::to_string(index.score));
                    }
                }
            }

            content << ";" << std::endl;
        }

        std::string data = compression::compress(content.str());
        std::ofstream file(file_name,std::ofstream::binary);

        file.write(data.data(), (int)data.size());
        file.close();
    }
}