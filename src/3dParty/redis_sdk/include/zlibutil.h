#pragma once


#include <zlib.h>
#include <vector>
#include <string>

inline bool GzipCompress(const std::string& input, std::string& output, int level = Z_BEST_COMPRESSION) {
    z_stream zs{};
    
    // 15 + 16:
    // 15 -> MAX_WBITS
    // +16 -> gzip header/trailer
    if (deflateInit2(
            &zs,
            Z_BEST_SPEED,
            Z_DEFLATED,
            15 + 16,
            8,
            Z_DEFAULT_STRATEGY) != Z_OK)
    {
        return false;
    }

    zs.next_in = (Bytef*)input.data();
    zs.avail_in = input.size();

    int ret;

    char outbuffer[32768];

    do
    {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (output.size() < zs.total_out)
        {
            output.append(
                outbuffer,
                zs.total_out - output.size());
        }

    } while (ret == Z_OK);

    deflateEnd(&zs);

    return ret == Z_STREAM_END;
}

inline bool GzipDecompress(const std::string& input, std::string& output, int level = Z_BEST_COMPRESSION) {
    z_stream zs{};

    // 15 + 32:
    // 自动识别 zlib/gzip header
    if (inflateInit2(&zs, 15 + 32) != Z_OK)
    {
        return false;
    }

    zs.next_in = (Bytef*)input.data();
    zs.avail_in = input.size();

    int ret;
    char outbuffer[32768];

    do
    {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (output.size() < zs.total_out)
        {
            output.append(
                outbuffer,
                zs.total_out - output.size());
        }

    } while (ret == Z_OK);

    inflateEnd(&zs);

    return ret == Z_STREAM_END;
}