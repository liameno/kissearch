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

#include "entry.h"

namespace kissearch {
    struct document {
    public:
        typedef std::pair<entry&, double> result_t;
        typedef std::pair<std::string, double> cache_idf_t;
    public:
        struct search_options {
            std::vector<std::string> field_names;
            double min_score;
            bool sort_by_score;
            ulong page;
            ulong page_size;

            search_options();
        };
    public:
        static std::string get_file_content(const std::string &file_name);

        static void normalize(std::string &s);
        static std::vector<std::string> tokenize(std::string &text);
        static void stem(std::vector<std::string> &terms);

        static void tokenize(field_text &field);
        static void stem(field_text &field);
        static void sort_text_results(std::vector<result_t> &results);
    public:
        std::string name;
        std::vector<entry> entries;
        std::vector<cache_idf_t> cache_idf;
    private:
        double k;
        double b;
        ulong cache_idf_size;
        std::mutex mutex;
    private:
        static void write_block(std::stringstream &content, const std::string &type, const std::string &value);
        static void write_block(std::stringstream &content, const std::string &key, const std::string &type, const std::string &value);
        static void parse_block(const std::string &s, std::string &key, std::string &type, std::string &value);
    public:
        explicit document(const ulong &cache_idf_size = 512, const double &k = 1.2, const double &b = 0.75);

        ulong compute_document_length_in_words(const std::string &field_name);
        double compute_tf(entry &e, const std::string &term, field_text &field);
        double compute_idf(entry &e, const std::string &term, const std::string &field_name);
        double compute_bm25(entry &e, const std::string &term, field_text &field, const std::string &field_name, const ulong &document_length_in_words);

        //ID
        ulong compute_next_number_value(const std::string &field_name);

        void clear_cache_idf();
        void index_text_field(const std::string &field_name);

        static void slice_page(std::vector<result_t> &results, const search_options &options);

        std::vector<result_t> number_search(const u_long &query, const search_options &options);
        std::vector<result_t> text_search(std::string query, const search_options &options);
        std::vector<result_t> keyword_search(const std::string &query, const search_options &options);

        void add(const entry &e);

        void load(const std::string &file_name);
        void save(const std::string &file_name);
    };
}

#endif