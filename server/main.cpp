#include <iostream>

#include "document.h"
#include "collection.h"

#include "include/httplib.h"
#include "include/json.hpp"
#include "str.h"

using namespace httplib;
using namespace nlohmann;
using namespace kissearch;

#define lambda_args const Request &req, Response &res
#define already_exists_document() { response["status"] = "error";\
                            response["message"] = "Already Exists";\
                            res.status = 404;\
                            res.set_content(response.dump(), "application/json");\
                            return; }
#define not_found_document() { response["status"] = "error";\
                            response["message"] = "Not Found";\
                            res.status = 404;\
                            res.set_content(response.dump(), "application/json");\
                            return; }
#define not_found_field() { response["status"] = "error";\
                        response["message"] = "Not Found Field";\
                        res.status = 500;\
                        res.set_content(response.dump(), "application/json");\
                        return; }

inline document::search_options parse_search_options(const json &params) {
    document::search_options options;

    for (auto &param : params.items()) {
        const auto &key = param.key();
        const auto &value = param.value();

        if (key == "field_names") {
            options.field_names = split(value, ",");
        } else if (key == "sort_by_score") {
            options.sort_by_score = (value == "1" || value == "true");
        } else if (key == "page") {
            options.page = value;
        } else if (key == "page_size") {
            options.page_size = value;
        }
    }

    return options;
}

int main() {
    Server server;
    collection collection;
    auto &documents = collection.documents;

    server.Get("/document/(\\w*)", [&](lambda_args) {
        auto &name = req.matches[1];
        auto found = collection.find_document(name);
        json response;

        if (found == documents.end()) not_found_document()
        auto doc = found->get();

        response["status"] = "ok";
        response["entries"]["count"] = doc->entries.size();

        for (auto &field : doc->fields) {
            json object;

            object["name"] = field.first;
            object["type"] = field.second;

            response["fields"].push_back(object);
        }

        res.status = 200;
        res.set_content(response.dump(), "application/json");
    });
    server.Post("/document/(\\w*)", [&](lambda_args) {
        auto &name = req.matches[1];
        auto found = collection.find_document(name);
        json response;

        if (found != documents.end()) already_exists_document()
        auto params = json::parse(req.body);

        ulong cache_idf_size = 512;
        double k = 1.2;
        double b = 0.75;

        if (params.find("cache_idf_size") != params.end()) cache_idf_size = params["cache_idf_size"];
        if (params.find("k") != params.end()) k = params["k"];
        if (params.find("b") != params.end()) b = params["b"];

        auto doc = std::make_shared<document>(cache_idf_size, k, b);
        doc->name = name;

        for (auto &param : params.items()) {
            doc->fields.emplace_back(param.key(), param.value());
        }

        collection.add(doc);

        response["status"] = "ok";
        res.status = 200;

        res.set_content(response.dump(), "application/json");
    });
    server.Post("/document/(\\w*)/add", [&](lambda_args) {
        auto &name = req.matches[1];
        auto found = collection.find_document(name);
        json response;

        if (found == documents.end()) not_found_document()
        auto doc = found->get();

        auto &fields = doc->fields;
        auto &entries = doc->entries;

        auto params = json::parse(req.body);
        entry e;

        for (auto &param : params.items()) {
            const auto &key = param.key();
            const auto &value = param.value();

            auto found2 = std::find_if(fields.begin(), fields.end(), [&](const document::field_t &c) { return c.first == key; });
            if (found2 == fields.end()) not_found_field()

            field f;
            f.name = key;

            if (found2->second == "number") f.val._number = std::make_shared<field::number>((std::string) value);
            else if (found2->second == "text") f.val._text = std::make_shared<field::text>((std::string) value);
            else if (found2->second == "keyword") f.val._keyword = std::make_shared<field::keyword>((std::string) value);
            else if (found2->second == "boolean") f.val._boolean = std::make_shared<field::boolean>((std::string) value);

            e.fields.push_back(f);

            if (found2->second == "text") doc->index_text_field(key);
        }

        for (auto &param : params.items()) {
            const auto &key = param.key();
            const auto &value = param.value();

            auto found2 = std::find_if(fields.begin(), fields.end(), [&](const document::field_t &c) { return c.first == key; });
            if (found2 == fields.end()) not_found_field()

            doc->entries.emplace_back(e);

            if (found2->second == "text") doc->index_text_field(key);
        }

        response["status"] = "ok";
        res.status = 200;

        res.set_content(response.dump(), "application/json");
    });
    server.Post("/document/(\\w*)/remove", [&](lambda_args) {
        auto &name = req.matches[1];
        auto found = collection.find_document(name);
        json response;

        if (found == documents.end()) not_found_document()
        auto doc = found->get();
        auto params = json::parse(req.body);
        auto options = parse_search_options(params);
        options.sort_by_score = false;

        auto results = doc->search((std::string) params["q"], options, true);

        for (const auto &result : results) {
            doc->remove(result.first);
        }

        response["status"] = "ok";
        response["count"] = results.size();
        res.status = 200;

        res.set_content(response.dump(), "application/json");
    });
    server.Post("/document/(\\w*)/search", [&](lambda_args) {
        auto &name = req.matches[1];
        auto found = collection.find_document(name);
        json response;

        if (found == documents.end()) not_found_document()
        auto doc = found->get();
        auto params = json::parse(req.body);
        auto options = parse_search_options(params);

        auto results = doc->search((std::string) params["q"], options);
        response["found"] = json::array();

        for (const auto &result : results) {
            json object = json::object();
            object["entry"] = json::object();

            for (auto &f : result.first.fields) {
                object["entry"][f.name] = f.val_s();
            }

            object["score"] = result.second;
            response["found"].push_back(object);
        }

        response["status"] = "ok";
        response["count"] = results.size();
        res.status = 200;

        res.set_content(response.dump(), "application/json");
    });

    server.listen("0.0.0.0", 8080);
}