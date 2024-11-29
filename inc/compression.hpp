#ifndef COMPRESSION_HPP
#define COMPRESSION_HPP

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

// Compress a tar file.
// src: The file or dir to be compress.
// dist: The archive file to be created (optional).
// type: The type of compression (GZ by default).
void compression_compress(const char *const src, const char *const tar = nullptr, const compression_type_t type = TAR_GZ);

// Decompress a tar file.
// src: The tar archive to be extracted.
// dir: The directory to be created (optional).
// type: The type of compression (autosearch, if empty).
void compression_extract(const char *const tar, const char *const dir = nullptr, const compression_type_t type = UNKNOWN);

#endif // COMPRESSION_HPP