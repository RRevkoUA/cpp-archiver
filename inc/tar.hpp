#ifndef TAR_HPP
#define TAR_HPP

// Compress a tar file.
// src: The file or dir to be compress.
// tar: The tar file to be created (optional).
void tar_compress(const char *const src, const char *const tar = nullptr);

// Decompress a tar file.
// tar: The tar file to be extracted.
// dir: The directory to be created (optional).
void tar_extract(const char *const tar, const char *const dir = nullptr);

#endif // TAR_HPP