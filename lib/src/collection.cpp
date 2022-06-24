#include "collection.h"

namespace kissearch {
    collection::collection() {
    }

    void collection::remove(const std::string &name) {
        mutex.lock();
        for (int i = 0; i < documents.size(); ++i) {
            if (documents[i]->name == name) {
                documents.erase(documents.begin() + i);
                --i;
            }
        }
        mutex.unlock();
    }
    void collection::add(const document_t &doc) {
        mutex.lock();
        this->documents.push_back(doc);
        mutex.unlock();
    }
}