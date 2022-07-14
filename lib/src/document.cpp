#include <fstream>
#include <iterator>

#include "../include/document.h"

#include "../include/str.h"
#include "../include/compression.h"

namespace kissearch {
    inline std::string document::get_file_content(const std::string &file_name) {
        std::ifstream stream(file_name);
        std::string buffer;

        stream.seekg(0, std::ios::end);
        buffer.resize(stream.tellg());
        stream.seekg(0);
        stream.read(const_cast<char *>(buffer.data()), (int) buffer.size());

        return buffer;
    }

    inline bool document::is_stop(const std::string &s) {
        const static std::string words[] = {
                "i", "me", "my", "myself", "we", "our", "ours", "ourselves", "you", "your", "yours", "yourself", "yourselves", "he", "him", "his", "himself", "she", "her", "hers", "herself", "it",
                "its", "itself", "they", "them", "their", "theirs", "themselves", "what", "which", "who", "whom", "this", "that", "these", "those", "am", "is", "are", "was", "were", "be", "been",
                "being", "have", "has", "had", "having", "do", "does", "did", "doing", "would", "should", "could", "ought", "i'm", "you're", "he's", "she's", "it's", "we're", "they're", "i've",
                "you've", "we've", "they've", "i'd", "you'd", "he'd", "she'd", "we'd", "they'd", "i'll", "you'll", "he'll", "she'll", "we'll", "they'll", "isn't", "aren't", "wasn't", "weren't",
                "hasn't", "haven't", "hadn't", "doesn't", "don't", "didn't", "won't", "wouldn't", "shan't", "shouldn't", "can't", "cannot", "couldn't", "mustn't", "let's", "that's", "who's", "what's",
                "here's", "there's", "when's", "where's", "why's", "how's", "a", "an", "the", "and", "but", "if", "or", "because", "as", "until", "while", "of", "at", "by", "for", "with", "about",
                "against", "between", "into", "through", "during", "before", "after", "above", "below", "to", "from", "up", "down", "in", "out", "on", "off", "over", "under", "again", "further",
                "then", "once", "here", "there", "when", "where", "why", "how", "all", "any", "both", "each", "few", "more", "most", "other", "some", "such", "no", "nor", "not", "only", "own", "same",
                "so", "than", "too", "very",
        };
        const static auto words_size = 174;

        for (short i = 0; i < words_size; ++i) {
            auto &w = words[i];

            if (s.front() == w.front() && s == w) {
                return true;
            }
        }

        return false;
    }

    inline void document::normalize(std::string &s) {
        to_lower(s);
        remove_special_chars(s);
    }
    inline std::vector<std::string> document::tokenize(const std::string &text) {
        auto terms = split(text, " ");

        for (auto &s : terms) {
            normalize(s);
        }

        return terms;
    }
    inline void document::stem(std::vector<std::string> &terms) {
        const auto lambda = [](const std::string &term) { return is_stop(term); };
        terms.erase(std::remove_if(terms.begin(), terms.end(), lambda), terms.end());

        for (auto &term : terms) {
            auto stemmed = sb_stemmer_stem(stemmer, (sb_symbol *) term.c_str(), (int) term.length());
            term = (const char *) stemmed;
        }
    }

    inline void document::write_block(std::stringstream &content, const std::string &type, const std::string &value) {
        content << type
                << '/'
                << value
                << std::endl;
    }
    inline void document::write_block(std::stringstream &content, const std::string &key, const std::string &type, const std::string &value) {
        content << key
                << '/'
                << type
                << '/'
                << value
                << std::endl;
    }
    inline void document::parse_block(const std::string &s, std::string &key, std::string &type, std::string &value) {
        int start = 0;

        while (true) {
            size_t end = s.find('/', start);

            if (end == std::string::npos) {
                break;
            }

            if (start == 0) {
                key = s.substr(start, end - start);
                start = (int) end + 1;
            } else {
                type = s.substr(start, end - start);
                start = (int) end + 1;
                break;
            }
        }

        if (type.empty()) {
            type = key;
            key.clear();
        }

        value = s.substr(start, s.size() - start);
    }

    document::document(const double &k, const double &b) {
        this->k = k;
        this->b = b;
        this->stemmer = sb_stemmer_new("english", nullptr);
    }
    document::~document() {
        sb_stemmer_delete(stemmer);
    }

    inline ulong document::compute_document_length_in_words(const std::string &field_name) {
        ulong size = 0;

        for (auto &i : term_index) {
            size += i.second.entries.size();
        }

        return size;
    }
    inline double document::compute_tf(entry &e, const std::string &term) {
        auto tf = 0;

        for (auto &i : term_index) {
            auto found = i.second.entries.find(&e);
            if (term == i.first && found != i.second.entries.end()) {
                tf += found->second.count;
            }
        }

        return tf;
    }
    void document::compute_idf(const ulong &entries_size) {
        for (auto &i : term_index) {
            auto size = i.second.entries.size();
            i.second.idf = std::log1p(((double) entries_size - (double) size + 0.5) / ((double) size + 0.5));
        }
    }
    inline double document::compute_bm25(entry &e, const std::string &term, const double &idf, const ulong &terms_length, const double &avgdl) {
        auto tf = compute_tf(e, term);
        return idf * (tf * (k + 1)) / (tf + k * (1 - b + b * terms_length / avgdl));
    }
    int document::compute_damerau_levenshtein_distance(std::string s, std::string v) {
        const ulong s_size = s.length();
        const ulong v_size = v.length();

        int d[s_size + 1][v_size + 1];
        int cost;

        for (int i = 0; i <= s_size; i++) d[i][0] = i;
        for (int j = 0; j <= v_size; j++) d[0][j] = j;
        for (int i = 1; i <= s_size; i++) {
            for(int j = 1; j<= v_size; j++) {
                if(s[i - 1] == v[j - 1]) cost = 0;
                else cost = 1;

                d[i][j] = std::min(d[i-1][j] + 1, std::min(d[i][j-1] + 1, d[i-1][j-1] + cost));

                if((i > 1) && (j > 1) && (s[i-1] == v[j-2]) && (s[i-2] == v[j-1])) {
                    d[i][j] = std::min( d[i][j], d[i-2][j-2] + cost);
                }
            }
        }

        return d[s_size][v_size];
    }

    ulong document::compute_next_number_value(const std::string &field_name) {
        if (entries.empty()) return 1;
        return entries.back().find_field(field_name)._number->value + 1;
    }

    void document::index() {
        for (auto &field : fields) {
            if (field.second == "text") {
                index_text_field(field.first);
            }
        }
    }
    void document::index_text_field(const std::string &field_name) {
        mutex.lock();

        //tokenize, stem
        for (auto &entry : entries) {
            auto &field = entry.find_field(field_name);
            auto terms = split(field._text->value, " ");

            for (auto &s : terms) {
                normalize(s);
            }

            const auto lambda = [](const std::string &term) { return is_stop(term); };
            terms.erase(std::remove_if(terms.begin(), terms.end(), lambda), terms.end());

            for (auto &term : terms) {
                auto stemmed = sb_stemmer_stem(stemmer, (sb_symbol *) term.c_str(), (int) term.length());
                term = (const char *) stemmed;
                ++term_index[term].entries[&entry].count;
            }

            field._text->terms_length = terms.size();
        }

        auto entries_size = entries.size();
        auto avgdl = (double) compute_document_length_in_words(field_name) / (double) entries_size;
        compute_idf(entries_size);

        for (auto &i : term_index) {
            for (auto &e : i.second.entries) {
                auto &field = e.first->find_field(field_name);
                e.second.score = compute_bm25(*e.first, i.first, i.second.idf, field._text->terms_length, avgdl);
            }
        }

        mutex.unlock();
    }

    inline void document::slice_page(std::vector<result_t> &results, const search_options &options) {
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

    std::vector<document::result_t> document::search(const std::string &query, const search_options &options, const bool is_all) {
        auto terms = tokenize(query);
        stem(terms);

        std::vector<result_t> results;
        results.reserve(options.page_size);

        for (const auto &field_name : options.field_names) {
            auto type = std::find_if(fields.begin(), fields.end(),[&](auto &f) { return f.first == field_name; })->second;

            if (type == "text") {
                for (auto &i : term_index) {
                    for (auto &term : terms) {
                        if (i.first.length() < options.text.word_min_size) continue;
                        auto &match_type = options.text._match_type;

                        if (match_type == options.text.match_type::strict) {
                            if (i.first != term) {
                                continue;
                            }
                        } else if (match_type == options.text.match_type::fuzzy) {
                            if (compute_damerau_levenshtein_distance(i.first, term) > options.text.fuzzy_max_damerau_levenshtein_distance) {
                                continue;
                            }
                        }

                        for (auto &entry : i.second.entries) {
                            auto &score = entry.second.score;
                            if (score <= 0) continue;

                            const auto lambda = [&](const result_t &c) { return c.first == entry.first; };
                            auto found = std::find_if(results.begin(), results.end(), lambda);

                            if (found != results.end()) found->second += score;
                            else results.emplace_back(entry.first, score);
                        }
                    }
                }
            } else {
                for (auto &entry : entries) {
                    auto &field = entry.find_field(field_name);
                    double score = 0;

                    if (type == "number") {
                        if (field._number->operator==(std::stol(query))) {
                            score = 1;
                        }
                    } else if (type == "keyword") {
                        if (field._keyword->operator==(query)) {
                            score = 1;
                        }
                    } else if (type == "boolean") {
                        if (field._boolean->operator==(query)) {
                            score = 1;
                        }
                    }

                    if (score <= 0) continue;

                    const auto lambda = [&](const result_t &c) { return c.first == &entry; };
                    auto found = std::find_if(results.begin(), results.end(), lambda);

                    if (found != results.end()) found->second += score;
                    else results.emplace_back(&entry, score);
                }
            }
        }

        if (options.sort_by_score) {
            std::sort(results.begin(), results.end(), [](const auto &x, const auto &y) {
                return x.second > y.second;
            });
        }
        if (!is_all) slice_page(results, options);

        return results;
    }

    void document::remove(const entry &e) {
        mutex.lock();
        for (int i = 0; i < entries.size(); ++i) {
            if (entries[i] == e) {
                entries.erase(entries.begin() + i);
                --i;
            }
        }
        mutex.unlock();
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

        std::string field_name;

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

            if (t == 'n') { //num field
                field f { key };
                f.val._number = std::make_shared<field::number>(value);
                e.fields.push_back(f);
            } else if (t == 't') { //text field
                field f { key };
                f.val._text = std::make_shared<field::text>(value);
                e.fields.push_back(f);
            } else if (t == 'k') { //keyword field
                field f { key };
                f.val._keyword = std::make_shared<field::keyword>(value);
                e.fields.push_back(f);
            } else if (t == 'b') { //boolean field
                field f { key };
                f.val._boolean = std::make_shared<field::boolean>(value);
                e.fields.push_back(f);
            } else if (t == 'm') { //global field name
                field_name = value;
            } else if (t == 'l') { //global field value
                fields.emplace_back(field_name, value);
            }
        }

        index();
    }
    void document::save(const std::string &file_name) {
        if (std::filesystem::exists(file_name)) {
            std::filesystem::remove(file_name);
        }

        std::stringstream content;
        write_block(content, "d", name);

        /*for (auto &i : term_index) {
            write_block(content, "i", i.first);
            write_block(content, "f", std::to_string(i.second.idf));

            for (auto &e : i.second.entries) {
                write_block(content, "e", i.first);
                write_block(content, "s", std::to_string(e.second.score));
                write_block(content, "c", std::to_string(e.second.count));
            }
        }*/

        for (auto &i : fields) {
            write_block(content, "m", i.first);
            write_block(content, "l", i.second);
        }

        for (auto &entry : entries) {
            for (auto &f : entry.fields) {
                std::string type;
                std::string value;

                if (f.val.is_number()) {
                    value = std::to_string(f.val._number->value);
                    type = "n";
                } else if (f.val.is_text()) {
                    value = f.val._text->value;
                    type = "t";
                } else if (f.val.is_keyword()) {
                    value = f.val._keyword->value;
                    type = "k";
                } else if (f.val.is_boolean()) {
                    value = std::to_string(f.val._boolean->value);
                    type = "b";
                }

                write_block(content, f.name, type, value);

                if (type == "t") {
                    write_block(content, "l", std::to_string(f.val._text->terms_length));
                }
            }

            content << ";" << std::endl;
        }

        std::string data = compression::compress(content.str());
        std::ofstream file(file_name, std::ofstream::binary);

        file.write(data.data(), (int) data.size());
        file.close();
    }
}