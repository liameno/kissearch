#include <cstring>
#include "../include/compression.h"

namespace kissearch::compression {
    std::string compress(const std::string &s, const int &level) {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));
        deflateInit(&zs, level);

        zs.next_in = (Bytef *) s.data();
        zs.avail_in = (uint) s.size();

        int code;
        char buffer[BLOCK_SIZE];
        std::string result;

        do {
            zs.next_out = reinterpret_cast<Bytef *>(buffer);
            zs.avail_out = BLOCK_SIZE;

            code = deflate(&zs, Z_FINISH);

            if (result.size() < zs.total_out) {
                result.append(buffer, zs.total_out - result.size());
            }
        } while (code == Z_OK);

        deflateEnd(&zs);
        return result;
    }
    std::string decompress(const std::string &s) {
        z_stream zs;
        memset(&zs, 0, sizeof(zs));
        inflateInit(&zs);

        zs.next_in = (Bytef *) s.data();
        zs.avail_in = (uint) s.size();

        int code;
        char buffer[BLOCK_SIZE];
        std::string result;

        do {
            zs.next_out = reinterpret_cast<Bytef *>(buffer);
            zs.avail_out = BLOCK_SIZE;

            code = inflate(&zs, 0);

            if (result.size() < zs.total_out) {
                result.append(buffer, zs.total_out - result.size());
            }
        } while (code == Z_OK);

        inflateEnd(&zs);
        return result;
    }
}