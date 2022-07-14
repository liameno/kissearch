#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <thread>
#include <mutex>
#include <libstemmer.h>

#include "entry.h"

namespace kissearch {
    class document {
    public:
        typedef std::pair<std::string, std::string> field_t;
        typedef std::pair<entry *, double> result_t;

        struct search_options {
            std::vector<std::string> field_names;
            bool sort_by_score = true;
            ulong page = 1;
            ulong page_size = 10;

            struct {
                enum match_type {
                    strict,
                    fuzzy,
                };

                ulong word_min_size = 3;
                ulong fuzzy_max_damerau_levenshtein_distance = 2;
                match_type _match_type = match_type::fuzzy;
            } text;
        };
        struct entry_info {
            double score = 0;
            ulong count = 0;
        };
        struct term_info {
            std::unordered_map<entry *, entry_info> entries;
            double idf = 0;
        };

        std::string name;
        std::vector<entry> entries;
        std::vector<field_t> fields;
        std::unordered_map<std::string, term_info> term_index;
    private:
        double k;
        double b;
        std::mutex mutex;
        struct sb_stemmer *stemmer;
    public:
        inline static std::string get_file_content(const std::string &file_name);

        inline static bool is_stop(const std::string &s);

        inline static void normalize(std::string &s);
        inline static std::vector<std::string> tokenize(const std::string &text);
        inline void stem(std::vector<std::string> &terms);
    private:
        inline static void write_block(std::stringstream &content, const std::string &type, const std::string &value);
        inline static void write_block(std::stringstream &content, const std::string &key, const std::string &type, const std::string &value);
        inline static void parse_block(const std::string &s, std::string &key, std::string &type, std::string &value);
    private:
        inline ulong compute_document_length_in_words(const std::string &field_name);
        inline double compute_tf(entry &e, const std::string &term);
        inline void compute_idf(const ulong &entries_size);
        inline double compute_bm25(entry &e, const std::string &term, const double &idf, const ulong &terms_length, const double &avgdl);
        int compute_damerau_levenshtein_distance(std::string s, std::string v);
    public:
        explicit document(const double &k = 1.2, const double &b = 0.75);
        ~document();

        //ID
        ulong compute_next_number_value(const std::string &field_name);

        void index();
        void index_text_field(const std::string &field_name);

        inline static void slice_page(std::vector<result_t> &results, const search_options &options);

        std::vector<result_t> search(const std::string &query, const search_options &options, const bool is_all = false);

        void remove(const entry &e);
        void add(const entry &e);

        void load(const std::string &file_name);
        void save(const std::string &file_name);
    };
}

#endif