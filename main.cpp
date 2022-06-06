#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>

#include "include/document.h"

void load_example(document &document, const std::string &field_number_name, const std::string &field_text_name, const std::string &field_keyword_name) {
    std::vector<std::pair<std::string, std::string>> texts = {
            {"The Hilltop algorithm is an algorithm used to find documents relevant to a particular keyword topic in news search", "https://en.wikipedia.org/wiki/Hilltop_algorithm"},
            {"VisualRank is a system for finding and ranking images by analysing and comparing their content, rather than searching image names, Web links or other text", "https://en.wikipedia.org/wiki/VisualRank"},
            {"TrustRank is an algorithm that conducts link analysis to separate useful webpages from spam and helps search engine rank pages in SERPs (Search Engine Results Pages)", "https://en.wikipedia.org/wiki/TrustRank"},
            {"The CheiRank is an eigenvector with a maximal real eigenvalue of the Google matrix G ∗ {\\displaystyle G^{*}} {\\displaystyle G^{*}} constructed for a directed network with the inverted directions of links", "https://en.wikipedia.org/wiki/CheiRank"},
            {"PageRank (PR) is an algorithm used by Google Search to rank web pages in their search engine results", "https://en.wikipedia.org/wiki/PageRank"},
            {"Okapi BM25 (BM is an abbreviation of best matching) is a ranking function used by search engines to estimate the relevance of documents to a given search query", "https://en.wikipedia.org/wiki/Okapi_BM25"},
            {"term frequency–inverse document frequency", "https://en.wikipedia.org/wiki/Tf%E2%80%93idf"},
    };

    for (const auto &text : texts) {
        size_t n;
        entry e;

        if (document.entries.empty()) n = 1;
        else n = document.entries.back().numbers[field_number_name].number + 1;

        e.numbers[field_number_name] = field_number(n);
        e.texts[field_text_name] = field_text(text.first);
        e.keywords[field_keyword_name] = field_keyword(text.second);

        document.add(e);
    }
}

int main() {
    using namespace std::chrono;

    const std::string file_name          = "../index.db";
    const std::string field_number_name  = "id";
    const std::string field_text_name    = "title";
    const std::string field_keyword_name = "url";

    const ulong       number_query  = 5;
    const std::string text_query    = "algorithm";
    const std::string keyword_query = "https://en.wikipedia.org/wiki/Hilltop_algorithm";

    document document;

    auto start_time = high_resolution_clock::now();

    if (!std::filesystem::exists(file_name)) {
        load_example(document, field_number_name, field_text_name, field_keyword_name);
        std::cout << "From example" << std::endl;
    } else {
        document.load(file_name);
        std::cout << "From db" << std::endl;
    }

    auto end_time = high_resolution_clock::now();
    std::cout << "Load: " << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;
    std::cout << "Size: " << document.entries.size() << std::endl;
    start_time = high_resolution_clock::now();

    document.index_text_field(field_text_name);

    end_time = high_resolution_clock::now();
    std::cout << "Index: " << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;
    start_time = high_resolution_clock::now();

    auto n_results = document.number_search(number_query, field_number_name);
    auto t_results = document.text_search(text_query, field_text_name);
    auto k_results = document.keyword_search(keyword_query, field_keyword_name);

    document::sort_text_results(t_results);

    for (auto &result : n_results) {
        auto &field = result.first.numbers[field_number_name];
        std::cout << field.number << " (score: " << result.second << ")" << std::endl;
    }
    for (auto &result : t_results) {
        auto &field = result.first.texts[field_text_name];
        std::cout << field.text << " (score: " << result.second << ")" << std::endl;
    }
    for (auto &result : k_results) {
        auto &field = result.first.keywords[field_keyword_name];
        std::cout << field.keyword << " (score: " << result.second << ")" << std::endl;
    }

    end_time = high_resolution_clock::now();
    std::cout << "Search: " << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;
    start_time = high_resolution_clock::now();

    document.save(file_name);

    end_time = high_resolution_clock::now();
    std::cout << "Save: " << duration_cast<milliseconds>(end_time - start_time).count() << " ms" << std::endl;

    return 0;
}
