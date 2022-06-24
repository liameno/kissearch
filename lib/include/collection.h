#ifndef COLLECTION_H
#define COLLECTION_H

#include "document.h"

namespace kissearch {
    class collection {
    public:
        typedef std::shared_ptr<document> document_t;
    private:
        std::mutex mutex;
    public:
        std::vector<document_t> documents;

        collection();

        void remove(const std::string &name);
        void add(const document_t &doc);

        inline auto find_document(const std::string &name) {
            const auto lambda = [&](const document_t &doc) { return doc->name == name; };
            return std::find_if(documents.begin(), documents.end(), lambda);
        }
    };
}

#endif