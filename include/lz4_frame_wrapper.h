
#include <exception>
#include <cstdio>
#include <malloc.h>
#include <lz4frame.h>

namespace lz4_frame_wrapper {
#define IN_CHUNK_SIZE  (16*1024)

    static const LZ4F_preferences_t k_prefs = {{
                LZ4F_max256KB,
             LZ4F_blockLinked,
             LZ4F_noContentChecksum,
             LZ4F_frame,
             0,
             0,
             LZ4F_noBlockChecksum
             },
            3, //level
            0,
            0,
            {0, 0, 0},
    };


/* safe_fwrite() :
 * performs fwrite(), ensure operation success, or immediately exit() */
    static void safe_fwrite(void *buf, size_t eltSize, size_t nbElt, FILE *f) {
        size_t const writtenSize = fwrite(buf, eltSize, nbElt, f);
        size_t const expectedSize = eltSize * nbElt;
        if (writtenSize < expectedSize) {
            throw std::exception();
        }
    }


/* ================================================= */
/*     Streaming Compression example               */
/* ================================================= */

    typedef struct {
        int error;
        unsigned long long size_in;
        unsigned long long size_out;
    } compressResult_t;

    static compressResult_t
    compress_file_internal(FILE *f_in, FILE *f_out, LZ4F_compressionContext_t ctx, void *in_buff, size_t in_chunk_size,
                           void *out_buff, size_t out_capacity) {
        compressResult_t result = {1, 0, 0};  /* result for an error */
        unsigned long long count_in = 0, count_out;

        /* write frame header */
        {
            size_t const headerSize = LZ4F_compressBegin(ctx, out_buff, out_capacity, &k_prefs);

            if (LZ4F_isError(headerSize)) {
                throw std::exception();
                return result;
            }

            count_out = headerSize;
            safe_fwrite(out_buff, 1, headerSize, f_out);
        }

        /* stream file */
        for (;;) {
            size_t const readSize = fread(in_buff, 1, IN_CHUNK_SIZE, f_in);
            if (readSize == 0) break; /* nothing left to read from input file */
            count_in += readSize;

            size_t const compressedSize = LZ4F_compressUpdate(ctx, out_buff, out_capacity, in_buff, readSize, nullptr);
            if (LZ4F_isError(compressedSize)) {
                throw std::exception();
                return result;
            }

            safe_fwrite(out_buff, 1, compressedSize, f_out);
            count_out += compressedSize;
        }

        /* flush whatever remains within internal buffers */
        {
            size_t const compressedSize = LZ4F_compressEnd(ctx, out_buff, out_capacity, nullptr);
            if (LZ4F_isError(compressedSize)) {
                throw std::exception();
                return result;
            }

            safe_fwrite(out_buff, 1, compressedSize, f_out);
            count_out += compressedSize;
        }

        result.size_in = count_in;
        result.size_out = count_out;
        result.error = 0;
        return result;
    }

    static compressResult_t compress_file(FILE *f_in, FILE *f_out) {
        /* resource allocation */
        LZ4F_compressionContext_t ctx;
        size_t const ctx_creation = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
        void *const src = malloc(IN_CHUNK_SIZE);
        size_t const outbuf_capacity = LZ4F_compressBound(IN_CHUNK_SIZE,
                                                          &k_prefs);   /* large enough for any input <= IN_CHUNK_SIZE */
        void *const outbuff = malloc(outbuf_capacity);

        compressResult_t result = {1, 0, 0};  /* == error (default) */
        if (!LZ4F_isError(ctx_creation) && src && outbuff) {
            result = compress_file_internal(f_in, f_out,
                                            ctx,
                                            src, IN_CHUNK_SIZE,
                                            outbuff, outbuf_capacity);
        } else {
            printf("error : resource allocation failed \n");
        }

        LZ4F_freeCompressionContext(ctx);   /* supports free on nullptr */
        free(src);
        free(outbuff);
        return result;
    }


/* ================================================= */
/*     Streaming decompression example               */
/* ================================================= */

    static size_t get_block_size(const LZ4F_frameInfo_t *info) {
        switch (info->blockSizeID) {
            case LZ4F_default:
            case LZ4F_max64KB:
                return 1 << 16;
            case LZ4F_max256KB:
                return 1 << 18;
            case LZ4F_max1MB:
                return 1 << 20;
            case LZ4F_max4MB:
                return 1 << 22;
            default:
                throw std::exception();
        }
    }

/* @return : 1==error, 0==success */
    static int
    decompress_file_internal(FILE *f_in, FILE *f_out, LZ4F_dctx *dctx, void *src, size_t src_capacity, size_t filled,
                             size_t already_consumed, void *dst, size_t dst_capacity) {
        int firstChunk = 1;
        size_t ret = 1;

        /* Decompression */
        while (ret != 0) {
            /* Load more input */
            size_t readSize = firstChunk ? filled : fread(src, 1, src_capacity, f_in);
            firstChunk = 0;
            const void *srcPtr = (const char *) src + already_consumed;
            already_consumed = 0;
            const void *const srcEnd = (const char *) srcPtr + readSize;
            if (readSize == 0 || ferror(f_in)) {
                throw std::exception();
            }

            /* Decompress:
             * Continue while there is more input to read (srcPtr != srcEnd)
             * and the frame isn't over (ret != 0)
             */
            while (srcPtr < srcEnd && ret != 0) {
                /* Any data within dst has been flushed at this stage */
                size_t dstSize = dst_capacity;
                size_t srcSize = (const char *) srcEnd - (const char *) srcPtr;
                ret = LZ4F_decompress(dctx, dst, &dstSize, srcPtr, &srcSize, /* LZ4F_decompressOptions_t */ nullptr);
                if (LZ4F_isError(ret)) {
                    throw std::exception();
                }
                /* Flush output */
                if (dstSize != 0) safe_fwrite(dst, 1, dstSize, f_out);
                /* Update input */
                srcPtr = (const char *) srcPtr + srcSize;
            }

            /* Ensure all input data has been consumed.
             * It is valid to have multiple frames in the same file,
             * but this example only supports one frame.
             */
            if (srcPtr < srcEnd) {
                throw std::exception();
            }
        }

        /* Check that there isn't trailing data in the file after the frame.
         * It is valid to have multiple frames in the same file,
         * but this example only supports one frame.
         */
        {
            size_t const readSize = fread(src, 1, 1, f_in);
            if (readSize != 0 || !feof(f_in)) {
                throw std::exception();
            }
        }

        return 0;
    }


/* @return : 1==error, 0==completed */
    static int decompress_file_alloc_dst(FILE *f_in, FILE *f_out, LZ4F_dctx *dctx, void *src, size_t src_capacity) {
        /* Read Frame header */
        size_t const readSize = fread(src, 1, src_capacity, f_in);
        if (readSize == 0 || ferror(f_in)) {
            throw std::exception();
        }

        LZ4F_frameInfo_t info;
        size_t consumedSize = readSize;
        {
            size_t const fires = LZ4F_getFrameInfo(dctx, &info, src, &consumedSize);
            if (LZ4F_isError(fires)) {
                throw std::exception();
            }
        }

        /* Allocating enough space for an entire block isn't necessary for
         * correctness, but it allows some memcpy's to be elided.
         */
        size_t const dstCapacity = get_block_size(&info);
        void *const dst = malloc(dstCapacity);
        if (!dst) {
            throw std::exception();
        }

        int const decompressionResult = decompress_file_internal(
                f_in, f_out,
                dctx,
                src, src_capacity, readSize - consumedSize, consumedSize,
                dst, dstCapacity);

        free(dst);
        return decompressionResult;
    }


/* @result : 1==error, 0==success */
    static int decompress_file(FILE *f_in, FILE *f_out) {
        /* Resource allocation */
        void *const src = malloc(IN_CHUNK_SIZE);
        if (!src) {
            throw std::exception();
        }

        LZ4F_dctx *dctx;
        {
            size_t const dctxStatus = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
            if (LZ4F_isError(dctxStatus)) {
                throw std::exception();
            }
        }

        int const result = !dctx ? 1 /* error */ :
                           decompress_file_alloc_dst(f_in, f_out, dctx, src, IN_CHUNK_SIZE);

        free(src);
        LZ4F_freeDecompressionContext(dctx);   /* note : free works on nullptr */
        return result;
    }

//int main(int argc, const char **argv) {
//    /* compress */
//
//    auto inpFilename = "../index.txt";
//    auto lz4Filename = "out";
//    auto decFilename = "out2";
//
//    {
//        FILE *const inpFp = fopen(inpFilename, "rb");
//        FILE *const outFp = fopen(lz4Filename, "wb");
//
//        compressResult_t const ret = compress_file(inpFp, outFp);
//
//        fclose(outFp);
//        fclose(inpFp);
//
//        if (ret.error) {
//            printf("compress : failed with code %i\n", ret.error);
//            return ret.error;
//        }
//        printf("%zu → %zu bytes, %.1f%%\n",
//               (size_t) ret.size_in,
//               (size_t) ret.size_out,  /* might overflow is size_t is 32 bits and size_{in,out} > 4 GB */
//               (double) ret.size_out / ret.size_in * 100);
//        printf("compress : done\n");
//    }
//
//    {
//        FILE *const inpFp = fopen(lz4Filename, "rb");
//        FILE *const outFp = fopen(decFilename, "wb");
//
//        printf("decompress : %s -> %s\n", lz4Filename, decFilename);
//        int const ret = decompress_file(inpFp, outFp);
//
//        fclose(outFp);
//        fclose(inpFp);
//
//        if (ret) {
//            printf("decompress : failed with code %i\n", ret);
//            return ret;
//        }
//        printf("decompress : done\n");
//    }
//
//    {
//        FILE *const inpFp = fopen(inpFilename, "rb");
//        FILE *const decFp = fopen(decFilename, "rb");
//
//        printf("verify : %s <-> %s\n", inpFilename, decFilename);
//        int const cmp = compareFiles(inpFp, decFp);
//
//        fclose(decFp);
//        fclose(inpFp);
//
//        if (cmp) {
//            printf("corruption detected : decompressed file differs from original\n");
//            return cmp;
//        }
//        printf("verify : OK\n");
//    }
//
//    return 0;
//}
}