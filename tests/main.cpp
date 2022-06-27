#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include <catch2/catch.hpp>

#include "document.h"
#include "collection.h"
#include "str.h"
#include "compression.h"

using namespace kissearch;

void load_example(document &document, const std::string &field_name_number, const std::string &field_name_text, const std::string &field_name_keyword, int count) {
    std::vector<std::pair<std::string, std::string>> texts = {
            { "The Hilltop algorithm is an algorithm used to find documents relevant to a particular keyword topic in news search",                                                    "https://en.wikipedia.org/wiki/Hilltop_algorithm" },
            { "VisualRank is a system for finding and ranking images by analysing and comparing their content, rather than searching image names, Web links or other text",            "https://en.wikipedia.org/wiki/VisualRank" },
            { "TrustRank is an algorithm that conducts link analysis to separate useful webpages from spam and helps search engine rank pages in SERPs (Search Engine Results Pages)", "https://en.wikipedia.org/wiki/TrustRank" },
            { "The CheiRank is an eigenvector with a maximal real eigenvalue of the Google matrix constructed for a directed network with the inverted directions of links",           "https://en.wikipedia.org/wiki/CheiRank" },
            { "PageRank (PR) is an algorithm used by Google Search to rank web pages in their search engine results",                                                                  "https://en.wikipedia.org/wiki/PageRank" },
    };

    for (int i = 0; i < count; ++i) {
        for (const auto &text : texts) {
            entry e;

            field f_n, f_t, f_k;

            f_n.name = field_name_number;
            f_n.val._number = std::make_shared<field::number>(document.compute_next_number_value(field_name_number));

            f_t.name = field_name_text;
            f_t.val._text = std::make_shared<field::text>(text.first);

            f_k.name = field_name_keyword;
            f_k.val._keyword = std::make_shared<field::keyword>(text.second);

            e.fields.push_back(f_n);
            e.fields.push_back(f_t);
            e.fields.push_back(f_k);

            document.add(e);
        }
    }

    document.name = "example";
}

TEST_CASE("Str", "[str]") {
    REQUIRE(starts_with("test", "tes"));
    REQUIRE(ends_with("test", "est"));
}
TEST_CASE("Compression", "[compression]") {
    const std::string s = "test";
    auto compressed = compression::compress(s);
    auto decompressed = compression::decompress(compressed);

    REQUIRE(!compressed.empty());
    REQUIRE(decompressed == s);
}
TEST_CASE("Document", "[document]") {
    const std::string file_name = "index.db";
    const std::string field_name_number = "id";
    const std::string field_name_text = "title";
    const std::string field_name_keyword = "url";

    const std::string number_query = "5";
    const std::string text_query = "algorithms link";
    const std::string keyword_query = "https://en.wikipedia.org/wiki/Hilltop_algorithm";

    collection collection;
    collection.documents.push_back(std::make_shared<document>());
    auto &document = *collection.documents.front();

    BENCHMARK("load from memory") {
        document.entries.clear();
        return load_example(document, field_name_number, field_name_text, field_name_keyword, 1000); //1000(count) * 5 = 5000(entries.size)
    };
    BENCHMARK("save") {
        return document.save(file_name);
    };
    BENCHMARK("load from db") {
        document.entries.clear();
        return document.load(file_name);
    };

    BENCHMARK("index") {
        return document.index_text_field(field_name_text);
    };

    REQUIRE(document.entries[0].fields.size() == 3);
    REQUIRE(document.entries[0].fields[1].val._text->terms.size() > 0);
    REQUIRE(document.entries[0].fields[1].val._text->index.size() > 0);

    document::search_options search_options_number;
    document::search_options search_options_text;
    document::search_options search_options_keyword;

    search_options_number.field_names = { field_name_number };
    search_options_text.field_names = { field_name_text };
    search_options_keyword.field_names = { field_name_keyword };

    BENCHMARK("number search") {
        auto results = document.search(number_query, search_options_number);
        REQUIRE(results.size() == 1);
    };
    BENCHMARK("text search") {
        auto results = document.search(text_query, search_options_text);
        REQUIRE(results.size() == 10); //10 - page size
    };
    BENCHMARK("keyword search") {
        auto results = document.search(keyword_query, search_options_keyword);
        REQUIRE(results.size() == 10); //10 - page size
    };
}