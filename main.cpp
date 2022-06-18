#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <zlib.h>
#include <cstring>

#include "include/document.h"

#define reset   "\033[0m"
#define black    "\033[30m"
#define red     "\033[31m"
#define green   "\033[32m"
#define yellow  "\033[33m"
#define blue    "\033[34m"
#define magenta "\033[35m"
#define cyan    "\033[36m"
#define white   "\033[37m"

using namespace kissearch;
using namespace std::chrono;

void load_example(document &document, const std::string &field_number_name, const std::string &field_text_name, const std::string &field_keyword_name) {
    std::vector<std::pair<std::string, std::string>> texts = {
            {"The Hilltop algorithm is an algorithm used to find documents relevant to a particular keyword topic in news search", "https://en.wikipedia.org/wiki/Hilltop_algorithm"},
            {"VisualRank is a system for finding and ranking images by analysing and comparing their content, rather than searching image names, Web links or other text", "https://en.wikipedia.org/wiki/VisualRank"},
            {"TrustRank is an algorithm that conducts link analysis to separate useful webpages from spam and helps search engine rank pages in SERPs (Search Engine Results Pages)", "https://en.wikipedia.org/wiki/TrustRank"},
            {"The CheiRank is an eigenvector with a maximal real eigenvalue of the Google matrix constructed for a directed network with the inverted directions of links", "https://en.wikipedia.org/wiki/CheiRank"},
            {"PageRank (PR) is an algorithm used by Google Search to rank web pages in their search engine results", "https://en.wikipedia.org/wiki/PageRank"},
            {"Okapi BM25 (BM is an abbreviation of best matching) is a ranking function used by search engines to estimate the relevance of documents to a given search query", "https://en.wikipedia.org/wiki/Okapi_BM25"},
            {"term frequency–inverse document frequency", "https://en.wikipedia.org/wiki/Tf%E2%80%93idf"},
    };

    for (int i = 0; i < 1000; ++i) {
        for (const auto &text: texts) {
            entry e;

            e.numbers.emplace_back(field_number_name, field_number(document.compute_next_number_value(field_number_name)));
            e.texts.emplace_back(field_text_name, field_text(text.first));
            e.keywords.emplace_back(field_keyword_name, field_keyword(text.second));

            document.add(e);
        }
    }
}

int main() {
    const std::string file_name          = "../index.db";
    const std::string field_number_name  = "id";
    const std::string field_text_name    = "title";
    const std::string field_keyword_name = "url";

    const ulong       number_query  = 5;
    const std::string text_query    = "algorithm";
    const std::string keyword_query = "https://en.wikipedia.org/wiki/Hilltop_algorithm";

    document document;
    document.name = "test";

    auto start_time = high_resolution_clock::now();

    if (!std::filesystem::exists(file_name)) {
        load_example(document, field_number_name, field_text_name, field_keyword_name);
        std::cout << reset << "From example" << std::endl;
    } else {
        document.load(file_name);
        std::cout << reset << "From db" << std::endl;
    }

    auto end_time = high_resolution_clock::now();
    std::cout << reset << "Load: " << cyan << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;
    std::cout << reset << "Size: " << red << document.entries.size() << std::endl;
    start_time = high_resolution_clock::now();

    document.index_text_field(field_text_name);

    end_time = high_resolution_clock::now();
    std::cout << reset << "Index: " << red << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;
    start_time = high_resolution_clock::now();

    auto n_results = document.number_search(number_query, field_number_name);
    auto t_results = document.text_search(text_query, field_text_name);
    auto k_results = document.keyword_search(keyword_query, field_keyword_name);

    end_time = high_resolution_clock::now();
    std::cout << reset << "Search: " << red << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;

    std::cout << reset << "Found Numbers: " << green << n_results.size() << std::endl;
    std::cout << reset << "Found Texts: " << green << t_results.size() << std::endl;
    std::cout << reset << "Found Keywords: " << green << k_results.size() << std::endl;

    document::sort_text_results(t_results);

    /*for (auto &result : n_results) {
        auto &field = result.first.find_field_number(field_number_name);
        std::cout << reset << field.number << green << " (score: " << result.second << ")" << std::endl;
    }
    for (auto &result : t_results) {
        auto &field = result.first.find_field_text(field_text_name);
        std::cout << reset << field.text << green << " (score: " << result.second << ")" << std::endl;
    }
    for (auto &result : k_results) {
        auto &field = result.first.find_field_keyword(field_keyword_name);
        std::cout << reset << field.keyword << green << " (score: " << result.second << ")" << std::endl;
    }*/

    start_time = high_resolution_clock::now();

    document.save(file_name);

    end_time = high_resolution_clock::now();
    std::cout << reset << "Save: " << cyan << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;

    return 0;
}
