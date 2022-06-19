#ifndef KISSEARCH_COMPRESSION_H
#define KISSEARCH_COMPRESSION_H

#include <string>
#include <zlib.h>

#define BLOCK_SIZE 4096

namespace kissearch::compression {
    std::string compress(const std::string &s, const int &level = Z_DEFAULT_COMPRESSION);
    std::string decompress(const std::string &s);
}

#endif