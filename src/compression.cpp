#include "compression.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <string.h>
#include <fcntl.h>
#include <filesystem>
#include <iostream>

#define BUFFER_SIZE (16 * 1024)
#define BLOCK_SIZE  (10 * 1024)

static void free(archive *write, archive *read);
static bool is_directory(const char *const path);
static compression_type_t get_compression_type(const char *const file);
static int8_t filter(archive *a, const compression_type_t type, filter_t filter);

// compress parts
// prepare the tar file
static int8_t compress_prepare(std::string *name, compression_type_t *use_type, const char *const src, const char *const tar);
// open the archive and disk
static int8_t compress_open(archive *a, archive *disk, const std::string name, const std::string src, const compression_type_t type);
// write the archive
static int8_t compress_writing(archive *a, archive *disk, const std::string src, const std::string tar);

//extract parts
// prepare the tar file
static int8_t extract_prepare(std::string *name, compression_type_t *use_type, const char *const src, const char *const dest);
// read the archive, and extract
static int8_t extract_reading(archive *a, archive *disk, const std::string dest, const char *const tar, const compression_type_t type);


int8_t compression_compress(const char *const src, const char *const tar, const compression_type_t type)
{
    archive *a = archive_write_new();
    archive *disk = archive_read_disk_new();
    std::string source_dir = src; 
    std::string tar_name = "";
    compression_type_t use_type = type;

    if (compress_prepare(&tar_name, &use_type, src, tar)) {
        std::cerr << "Error: Could not set archive name" << std::endl;
        return -1;
    }

    std::cout << "tar_file: " << tar_name << std::endl;

    if (compress_open(a, disk, tar_name, source_dir, use_type)) {
        std::cerr << "Error: Could not open archive" << std::endl;
        free(a, disk);
        return -1;
    }

    if (compress_writing(a, disk, source_dir, tar_name)) {
        std::cerr << "Error: Could not write archive" << std::endl;
        free(a, disk);
        return -1;
    }

    free(a, disk);
    std::cout << "Tar file created successfully: " << tar_name << std::endl;
    return 0;
}

int8_t compression_extract(const char *const tar, const char *const dest, const compression_type_t type)
{
    archive *a = archive_read_new();
    archive *disk = archive_write_disk_new();
    compression_type_t use_type = type; 
    std::string destination = "";

    if (extract_prepare(&destination, &use_type, tar, dest)) {
        std::cerr << "Error: Could not set destination folder" << std::endl;
        return -1;
    }

    std::cout << "destination folder is: " << destination << std::endl;

    if (extract_reading(a, disk, destination, tar, use_type)) {
        std::cerr << "Error: Could not read archive" << std::endl;
        free(disk, a);
        return -1;
    }

    std::cout << "files extracted successfully" << std::endl;
    free(disk, a);
    return 0;
}

// Static functions
// close and free archive objects
static void free(archive *write, archive *read)
{
    std::cout << "Close archive objects:";

    if (write) {
        std::cout << " archive";
        archive_write_close(write);
        archive_write_free(write);
        write = nullptr;
    }

    if (read) {
        std::cout << " disk";
        archive_read_close(read);
        archive_read_free(read);
        read = nullptr;
    }

    std::cout << std::endl;
}

// check if path is a directory
static bool is_directory(const char *const path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}

// autodetect compression type
static compression_type_t get_compression_type(const char *const file)
{
    if (!file) {
        return UNKNOWN;
    }

    std::filesystem::path path(file);

    for (auto &ext : compression_map) {
        if (path.extension() == ext.second) {
            return ext.first;
        }
    }
    return UNKNOWN;
}

// filter archive
static int8_t filter(archive *a, const compression_type_t type, filter_t filter)
{
    switch (type)
    {
    case TAR_GZ:
        if (filter == READ) {
            archive_read_support_filter_gzip(a);
        }
        else {
            archive_write_add_filter_gzip(a);
        }
        break;
    case TAR_BZ2:
        if (filter == READ) {
            archive_read_support_filter_bzip2(a);
        }
        else {
            archive_write_add_filter_bzip2(a);
        }
        break;
    case TAR_XZ:
        if (filter == READ) {
            archive_read_support_filter_xz(a);
        }
        else {
            archive_write_add_filter_xz(a);
        }
        break;
    case TAR_ZST:
        if (filter == READ) {
            archive_read_support_filter_zstd(a);
        }
        else {
            archive_write_add_filter_zstd(a);
        }
        break;
    case TAR_LZ4:
        if (filter == READ) {
            archive_read_support_filter_lz4(a);
        }
        else {
            archive_write_add_filter_lz4(a);
        }
        break;
    case TAR_LZMA:
        if (filter == READ) {
            archive_read_support_filter_lzma(a);
        }
        else {
            archive_write_add_filter_lzma(a);
        }
        break;
    case TAR_ZIP:
        if (filter == READ) {
            archive_read_support_format_zip(a);
        }
        else {
            archive_write_set_format_zip(a);
        }
        break;
    case TAR_RAR:
        if (filter == READ) {
            archive_read_support_format_rar(a);
        }
        else {
            std::cerr << "Error: RAR compression not supported" << std::endl;
            return -1;
        }
        break;
    case TAR_7Z:
        if (filter == READ) {
            archive_read_support_format_7zip(a);
        }
        else {
            archive_write_set_format_7zip(a);
        }
        break;
    default:
        std::cerr << "Error: Unknown compression type" << std::endl;
        return -1;
    }
    return 0;
}

// set archive name
static int8_t compress_prepare(std::string *name, compression_type_t *use_type, const char *const src, const char *const tar)
{
    if (!src || strlen(src) == 0 || !std::filesystem::exists(src)) {
        std::cerr << "Error: Invalid source directory" << std::endl;
        return -1;
    }

    if (*use_type == UNKNOWN) {
        *use_type = get_compression_type(tar);
        if (*use_type == UNKNOWN) {
            std::cout << "type was not set. Use tar.gz by default" << std::endl;
            *use_type = TAR_GZ;
        }
    }
   
    if (tar == nullptr) {
        std::cout<<"tar file not provided. Creating tar file with default name"<<std::endl;
        std::filesystem::path path = realpath(src, nullptr);

        *name = path.filename().string();
        *name += ".tar.";
        *name += compression_map[*use_type];
    }
    else if (is_directory(tar)) {
        std::string dir;
        dir = realpath(tar, nullptr);
        std::filesystem::path path = realpath(src, nullptr);
        *name = dir;
        *name += "/";
        *name += path.filename().string();
        *name += ".tar.";
        *name += compression_map[*use_type];
    }
    else {
        *name = tar;
    }
    return 0;
}

// open archive and disk
static int8_t compress_open(archive *a, archive *disk, const std::string name, const std::string src, const compression_type_t type)
{
    if (!a || !disk) {
        std::cerr << "Error: Could not create archive structures" << std::endl;
        return -1;
    }

    if (filter(a, type, WRITE)) {
        std::cerr << "Error: Could not set filter" << std::endl;
        return -1;
    }
    archive_write_set_format_ustar(a);

    if (archive_write_open_filename(a, name.c_str())) {
        std::cerr << "Error: " << archive_error_string(a) << std::endl;
        return -1;
    }

    archive_read_disk_set_standard_lookup(disk);

    if (archive_read_disk_open(disk, src.c_str())) {
        std::cerr << "Error: " << archive_error_string(disk) <<  std::endl;
        return -1;
    }

    return 0;
}

// write archive
static int8_t compress_writing(archive *a, archive *disk, std::string src, const std::string tar)
{
        archive_entry *entry = nullptr;
        ssize_t len;
        std::string src_path = src;

        int status = ARCHIVE_OK;
        int fd = 0;
        char buff[BUFFER_SIZE];

        while (true) {
        entry = archive_entry_new();
        status = archive_read_next_header2(disk, entry);

        if (status == ARCHIVE_EOF) break;
        
        if (status) {
            std::cerr << "Error: " << archive_error_string(disk) << std::endl;
            return -1;
        }

        src_path = archive_entry_pathname(entry);
        
        if (is_directory(src.c_str())) {
            src_path.erase(0, strlen(src.c_str()));
        } else {
            src_path = src_path.substr(src_path.find_last_of("/") + 1);
        }

        archive_entry_set_pathname(entry, src_path.c_str());

        archive_read_disk_descend(disk);
        if (archive_write_header(a, entry)) {
            std::cerr << "Error: " << archive_error_string(a) << std::endl;
            std::cerr << "Skip this entry" << std::endl;
            archive_entry_free(entry);
            continue;
        }

        if (archive_entry_size(entry) > 0) {
            fd = open(archive_entry_sourcepath(entry), O_RDONLY);
            if (fd >= 0) {
                while ((len = read(fd, buff, sizeof(buff))) > 0) {
                    archive_write_data(a, buff, len);
                }
                close(fd);
            }
        }
        archive_entry_free(entry);
    }
    return 0;
}

// prepare the tar file
static int8_t extract_prepare(std::string *name, compression_type_t *use_type, const char *const src, const char *const dest)
{
    if (!src || strlen(src) == 0 || !std::filesystem::exists(src)) {
        std::cerr << "Error: Invalid source directory" << std::endl;
        return -1;
    }

    if (*use_type == UNKNOWN) {
        *use_type = get_compression_type(src);
        if (*use_type == UNKNOWN) {
            std::cerr << "Error: Could not detect compression type" << std::endl;
            return -1;
        }
    }

    if (dest == nullptr) {
        std::cout << "destination folder not provided. Extracting to current directory" << std::endl;
        *name = "./";
    }
    else {
        *name = dest;
    }

    return 0;
}

static int8_t extract_reading(archive *a, archive *disk, const std::string dest, const char *const tar, const compression_type_t type)
{
    archive_entry *entry = nullptr;
    size_t size;
    int64_t offset;
    std::string new_path;
    int status = ARCHIVE_OK;
    const void *buff;

    if(!std::filesystem::exists(dest)){
        std::cerr << "Error: Destination folder does not exist" << std::endl;
        return -1;
    }
    
    filter(a, type, READ);
    archive_read_support_format_tar(a);

    if (status = archive_read_open_filename(a, tar, BLOCK_SIZE)) {
        std::cerr<< "Error: " << archive_error_string(a);
        return -1;
    }

    while (true) {
        status = archive_read_next_header(a, &entry);
        if (status == ARCHIVE_EOF) {
            break;
        } 
        else if (status) {
            std::cerr<< "Error: " << archive_error_string(a);
            return -1;
        } 

        // extract
        new_path = dest + "/" + archive_entry_pathname(entry);
        archive_entry_set_pathname(entry, new_path.c_str());

        if (archive_write_header(disk, entry)) {
            std::cerr<< "Error: " << archive_error_string(disk);
            return -1;
        }

        while (true) {
            status = archive_read_data_block(a, &buff, &size, &offset);
            if (status == ARCHIVE_EOF) break;
            else if (status) break;

            if (archive_write_data_block(disk, buff, size, offset)) {
                std::cerr<< "Error: " << archive_error_string(disk);
                return -1;
            }
        }

        if (archive_write_finish_entry(disk)) {
            std::cerr<< "Error: " << archive_error_string(disk);
            return -1;
        }
    }
    return 0;
}