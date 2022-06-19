#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>

#include <kissearch/document.h>

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

void load_example(document &document, const std::string &field_name_number, const std::string &field_name_text, const std::string &field_name_keyword) {
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

            e.numbers.emplace_back(field_name_number, field_number(document.compute_next_number_value(field_name_number)));
            e.texts.emplace_back(field_name_text, field_text(text.first));
            e.keywords.emplace_back(field_name_keyword, field_keyword(text.second));

            document.add(e);
        }
    }

    document.name = "example";
}

int main() {
    const std::string file_name          = "../index.db";
    const std::string field_name_number  = "id";
    const std::string field_name_text    = "title";
    const std::string field_name_keyword = "url";

    const ulong       number_query  = 5;
    const std::string text_query    = "algorithms link";
    const std::string keyword_query = "https://en.wikipedia.org/wiki/Hilltop_algorithm";

    document document;

    auto start_time = high_resolution_clock::now();

    if (!std::filesystem::exists(file_name)) {
        load_example(document, field_name_number, field_name_text, field_name_keyword);
        std::cout << reset << "From example" << std::endl;
    } else {
        document.load(file_name);
        std::cout << reset << "From db" << std::endl;
    }

    auto end_time = high_resolution_clock::now();
    std::cout << reset << "Load: " << cyan << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;
    std::cout << reset << "Size: " << red << document.entries.size() << std::endl;
    start_time = high_resolution_clock::now();

    document.index_text_field(field_name_text);

    end_time = high_resolution_clock::now();
    std::cout << reset << "Index: " << red << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;
    start_time = high_resolution_clock::now();

    document::search_options search_options_number;
    document::search_options search_options_text;
    document::search_options search_options_keyword;

    search_options_number.field_names = { field_name_number };
    search_options_text.field_names = { field_name_text };
    search_options_keyword.field_names = { field_name_keyword };

    auto n_results = document.number_search(number_query, search_options_number);
    auto t_results = document.text_search(text_query, search_options_text);
    auto k_results = document.keyword_search(keyword_query, search_options_keyword);

    end_time = high_resolution_clock::now();
    std::cout << reset << "Search: " << red << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;

    std::cout << reset << "Found Numbers: " << green << n_results.size() << std::endl;
    std::cout << reset << "Found Texts: " << green << t_results.size() << std::endl;
    std::cout << reset << "Found Keywords: " << green << k_results.size() << std::endl;

    /*for (auto &result : n_results) {
        auto &field = result.first.find_field_number(field_name_number);
        std::cout << reset << field.number << green << " (score: " << result.second << ")" << std::endl;
    }*/
    /*for (auto &result : t_results) {
        auto &field_id = result.first.find_field_number(field_name_number);
        auto &field = result.first.find_field_text(field_name_text);
        std::cout << magenta << field_id.number << reset << " - " << reset << field.text << green << " (score: " << result.second << ")" << std::endl;
    }*/
    /*for (auto &result : k_results) {
        auto &field = result.first.find_field_keyword(field_name_keyword);
        std::cout << reset << field.keyword << green << " (score: " << result.second << ")" << std::endl;
    }*/

    start_time = high_resolution_clock::now();

    document.save(file_name);

    end_time = high_resolution_clock::now();
    std::cout << reset << "Save: " << cyan << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;

    return 0;
}