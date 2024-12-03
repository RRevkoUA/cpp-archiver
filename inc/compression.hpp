#ifndef COMPRESSION_HPP
#define COMPRESSION_HPP
#include "compression_enums.hpp"

// Compress a tar file.
// src: The file or dir to be compress.
// dist: The archive file to be created (optional).
// type: The type of compression (GZ by default).
// return: 0 if success, -1 if error.
int8_t compression_compress(const char *const src, const char *const tar = nullptr, const compression_type_t type = TAR_GZ);

// Decompress a tar file.
// src: The tar archive to be extracted.
// dir: The directory to be created (optional).
// type: The type of compression (autosearch, if empty).
// return: 0 if success, -1 if error.
int8_t compression_extract(const char *const tar, const char *const dir = nullptr, const compression_type_t type = UNKNOWN);

#endif // COMPRESSION_HPP