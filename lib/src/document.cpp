#include "../include/document.h"

#include "../include/porter2.h"
#include "../include/compression.h"

namespace kissearch {
    document::search_options::search_options() {
        this->min_score = 0.1;
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
    void document::tokenize(field_text &field) {
        field.terms = tokenize(field.text);
    }
    void document::stem(field_text &field) {
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
            auto &field = entry.find_field_text(field_name);
            size += field.terms.size();
        }

        return size;
    }
    double document::compute_tf(entry &e, const std::string &term, field_text &field) {
        auto score = 0;

        for (auto &word: field.terms) {
            field.find_index(word).count = 0;
        }
        for (auto &w : field.terms) {
            if (term == w) {
                score++;
            }
        }

        return score;
    }
    double document::compute_idf(entry &e, const std::string &term, const std::string &field_name) {
        const static auto lambada = [&](const std::pair<std::string, double> &c) { return c.first == term; };
        auto found_cache = std::find_if(cache_idf.begin(), cache_idf.end(), lambada);
        if (found_cache != cache_idf.end()) return found_cache->second;

        const auto size = entries.size();
        ulong found_size = 0;

        for (auto &entry2: entries) {
            auto &field = entry2.find_field_text(field_name);

            if (field.find_term_it(term) != field.terms.end()) {
                ++found_size;
            }
        }

        auto idf = std::log(((double)size - (double)found_size + 0.5) / ((double)found_size + 0.5) + 1);
        cache_idf.emplace_back(term, idf);
        return idf;
    }
    double document::compute_bm25(entry &e, const std::string &term, field_text &field, const std::string &field_name, const ulong &document_length_in_words) {
        auto entries_size = (double)entries.size();
        auto field_size = (double)field.terms.size();

        auto tf = compute_tf(e, term, field);
        auto idf = compute_idf(e, term, field_name);
        auto avgdl = (double)document_length_in_words / entries_size;

        return idf * (tf * (k + 1)) / (tf + k * (1 - b + b * field_size / avgdl));
    }

    ulong document::compute_next_number_value(const std::string &field_name) {
        if (entries.empty()) return 1;
        return entries.back().find_field_number(field_name).number + 1;
    }

    void document::clear_cache_idf() {
        cache_idf = std::vector<cache_idf_t>(cache_idf_size);
    }
    void document::index_text_field(const std::string &field_name) {
        mutex.lock();
        clear_cache_idf();

        //tokenize, stem
        for (auto &entry : entries) {
            auto &field = entry.find_field_text(field_name);

            tokenize(field);
            stem(field);
        }

        const auto document_length_in_words = compute_document_length_in_words(field_name);

        //score
        for (auto &entry : entries) {
            auto &field = entry.find_field_text(field_name);

            for (const auto &term : field.terms) {
                field.find_index(term).score = compute_bm25(entry, term, field, field_name, document_length_in_words);
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

    std::vector<document::result_t> document::number_search(const ulong &query, const search_options &options) {
        const auto page_size_max = options.page * options.page_size;

        std::vector<result_t> results;

        for (const auto &field_name : options.field_names) {
            for (auto &entry: entries) {
                if (results.size() >= page_size_max) break;

                auto &field = entry.find_field_number(field_name);
                double score = 0;

                if (field.number == query) {
                    score = 1;
                }

                if (score < options.min_score) continue;
                results.emplace_back(entry, score);
            }
        }

        slice_page(results, options);
        return results;
    }
    std::vector<document::result_t> document::text_search(std::string query, const search_options &options) {
        const auto page_size_max = options.page * options.page_size;

        auto terms = tokenize(query);
        stem(terms);

        std::vector<result_t> results;

        for (const auto &field_name : options.field_names) {
            for (auto &entry: entries) {
                if (results.size() >= page_size_max) break;

                auto &field = entry.find_field_text(field_name);
                double score = 0;

                for (auto &term: terms) {
                    auto found = field.find_index_it(term);

                    if (found != field.index.end()) {
                        score += found->second.score;
                    }
                }

                if (score < options.min_score) continue;

                const auto lambada = [&](const result_t &c) { return c.first == entry; };
                auto found = std::find_if(results.begin(), results.end(), lambada);

                if (found != results.end()) found->second += score;
                else results.emplace_back(entry, score);
            }
        }

        slice_page(results, options);
        if (options.sort_by_score) sort_text_results(results);
        return results;
    }
    std::vector<document::result_t> document::keyword_search(const std::string &query, const search_options &options) {
        const auto page_size_max = options.page * options.page_size;

        std::vector<result_t> results;

        for (const auto &field_name : options.field_names) {
            for (auto &entry: entries) {
                if (results.size() >= page_size_max) break;

                auto &field = entry.find_field_keyword(field_name);

                if (field.keyword == query) {
                    results.emplace_back(entry, 1);
                }
            }
        }

        slice_page(results, options);
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
                e.numbers.emplace_back(key, field_number(value));
            }
            else if (t == 't') {
                e.texts.emplace_back(key, field_text(value));
                text_field = key;
            }
            else if (t == 'w') {
                e.find_field_text(text_field).terms.push_back(value);
                index_field = value;
            }
            else if (t == 'c') {
                e.find_field_text(text_field).find_index(index_field).count = std::stol(value);
            }
            else if (t == 's') {
                e.find_field_text(text_field).find_index(index_field).score = std::stof(value);
            }
            else if (t == 'k') {
                e.keywords.emplace_back(key, field_keyword(value));
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
            for (auto &number : entry.numbers) {
                write_block(content, number.first, "n", std::to_string(number.second.number));
            }
            for (auto &text : entry.texts) {
                write_block(content, text.first, "t", text.second.text);

                for (auto &term : text.second.terms) {
                    auto &index = text.second.find_index(term);
                    write_block(content, "w", term);
                    write_block(content, "c", std::to_string(index.count));
                    write_block(content, "s", std::to_string(index.score));
                }
            }
            for (auto &keyword : entry.keywords) {
                write_block(content, keyword.first, "k", keyword.second.keyword);
            }

            content << ";" << std::endl;
        }

        std::string data = compression::compress(content.str());
        std::ofstream file(file_name,std::ofstream::binary);

        file.write(data.data(), (int)data.size());
        file.close();
    }
}