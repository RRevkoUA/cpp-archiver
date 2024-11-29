#ifndef COMPRESSION_ENUMS_HPP
#define COMPRESSION_ENUMS_HPP
#include <map>
#include <string>

typedef enum {
    TAR_GZ = 0,
    TAR_BZ2,
    TAR_XZ,
    TAR_ZST,
    TAR_LZ4,
    TAR_LZMA,
    TAR_ZIP,
    TAR_RAR,
    TAR_7Z,
    UNKNOWN = -1
} compression_type_t; 

typedef enum {
    READ = 0,
    WRITE
} filter_t;

static std::map<compression_type_t, std::string> compression_map = {
    {TAR_GZ, "gz"},
    {TAR_BZ2, "bz2"},
    {TAR_XZ, "xz"},
    {TAR_ZST, "zst"},
    {TAR_LZ4, "lz4"},
    {TAR_LZMA, "lzma"},
    {TAR_ZIP, "zip"},
    {TAR_RAR, "rar"},
    {TAR_7Z, "7z"}
};

#endif // COMPRESSION_ENUMS_HPP