#include "../include/document.h"

#include "../include/porter2.h"
#include "../include/compression.h"

namespace kissearch {
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

    void document::load_parse(const std::string &s, std::string &key, std::string &type, std::string &value) {
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
        this->cache_idf_size = cache_idf_size ;
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

    void document::clear_cache_idf() {
        cache_idf = std::vector<cache_idf_t>(cache_idf_size);
    }

    void document::index_text_field(const std::string &field_name) {
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
    }

    std::vector<document::result_t> document::number_search(const ulong &query, const std::string &field_name) {
        std::vector<result_t> results;

        for (auto &entry : entries) {
            auto &field = entry.find_field_number(field_name);

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
            auto &field = entry.find_field_text(field_name);
            double score = 0;

            for (auto &term : terms) {
                auto found = field.find_index_it(term);

                if (found != field.index.end()) {
                    score += found->second.score;
                }
            }

            if (score < MIN_TEXT_SEARCH_SCORE) continue;
            results.emplace_back(entry, score);
        }

        return results;
    }
    std::vector<document::result_t> document::keyword_search(const std::string &query, const std::string &field_name) {
        std::vector<result_t> results;

        for (auto &entry : entries) {
            auto &field = entry.find_field_keyword(field_name);

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
        if (!entries.empty()) {
            entries.clear();
        }

        auto decompressed = compression::decompress(get_file_content(file_name));
        std::stringstream stream(decompressed);

        std::string s;
        entry e;

        std::string text_field;
        std::string index_field;

        while (getline(stream, s)) {
            if (s == ";") {
                add(e);
                e = entry();
                continue;
            }

            std::string key;
            std::string type;
            std::string value;

            load_parse(s, key, type, value);

            if (type == "n") {
                e.numbers.emplace_back(key, field_number(value));
            }
            else if (type == "t") {
                e.texts.emplace_back(key, field_text(value));
                text_field = key;
            }
            else if (type == "w") {
                e.find_field_text(text_field).terms.push_back(value);
                index_field = value;
            }
            else if (type == "c") {
                e.find_field_text(text_field).find_index(index_field).count = std::stol(value);
            }
            else if (type == "s") {
                e.find_field_text(text_field).find_index(index_field).score = std::stof(value);
            }
            else if (type == "k") {
                e.keywords.emplace_back(key, field_keyword(value));
            }
            else if (type == "db") {
                name = value;
            }
        }
    }
    void document::save(const std::string &file_name) {
        if (std::filesystem::exists(file_name)) {
            std::filesystem::remove(file_name);
        }

        std::stringstream content;
        content << "db"
                << '/'
                << name
                << std::endl;

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
                            << text.second.find_index(term).count
                            << std::endl;
                    content << "s"
                            << '/'
                            << text.second.find_index(term).score
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

        std::string data = compression::compress(content.str());
        std::ofstream file(file_name,std::ofstream::binary);

        file.write(data.data(), (int)data.size());
        file.close();
    }
}