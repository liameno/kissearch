#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <filesystem>

#include "entry.h"

namespace kissearch {
    struct document {
    public:
        typedef std::pair<entry&, double> result_t;
        typedef std::pair<std::string, double> cache_idf_t;
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
    public:
        explicit document(const ulong &cache_idf_size = 512, const double &k = 1.2, const double &b = 0.75);

        ulong compute_document_length_in_words(const std::string &field_name);
        double compute_tf(entry &e, const std::string &term, field_text &field);
        double compute_idf(entry &e, const std::string &term, const std::string &field_name);
        double compute_bm25(entry &e, const std::string &term, field_text &field, const std::string &field_name, const ulong &document_length_in_words);

        void clear_cache_idf();

        void index_text_field(const std::string &field_name);

        std::vector<result_t> number_search(const u_long &query, const std::string &field_name);
        std::vector<result_t> text_search(std::string query, const std::string &field_name);
        std::vector<result_t> keyword_search(const std::string &query, const std::string &field_name);

        void add(const entry &e);

        void load(const std::string &file_name);
        void save(const std::string &file_name);
    };
}

#endif