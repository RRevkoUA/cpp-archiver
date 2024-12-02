#include "compression.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <string.h>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <map>

#define BUFFER_SIZE (16 * 1024)
#define BLOCK_SIZE  (10 * 1024)

static void free(archive *write, archive *read);
static compression_type_t get_compression_type(const char *const file);
static int8_t filter(archive *a, const compression_type_t type, filter_t filter);

void compression_compress(const char *const src, const char *const tar, const compression_type_t type)
{
    archive *a = archive_write_new();
    archive *disk = archive_read_disk_new();
    archive_entry *entry = nullptr;
   
    std::string archive_name = src; 
    ssize_t len;
    int status = ARCHIVE_OK;
    int fd = 0;
    char buff[BUFFER_SIZE];
    char tar_file[FILENAME_MAX];


    if (!src || strlen(src) == 0) {
        std::cerr << "Error: Invalid source directory" << std::endl;
        return;
    }


    if (tar == nullptr || strcmp(tar, ".") == 0) {
        char cwd[FILENAME_MAX];
        if (!getcwd(cwd, sizeof(cwd))) {
            std::cerr << "Error: Could not get current directory" << std::endl;
            return;
        }

        archive_name.erase(archive_name.find_last_not_of("/") + 1);
        std::filesystem::path path = (archive_name == ".") ? cwd : archive_name;
        snprintf(tar_file, sizeof(tar_file), "%s/%s.tar.%s", cwd, path.filename().c_str(), compression_map[type].c_str());
    }
    else {
        strncpy(tar_file, tar, FILENAME_MAX);
    }

    std::cout << "tar_file: " << tar_file << std::endl;

    if (!a || !disk) {
        std::cerr << "Error: Could not create archive structures" << std::endl;
        free(a, disk);
        return;
    }

    if (!filter(a, type, WRITE)) {
        std::cerr << "Error: Could not set filter" << std::endl;
        free(a, disk);
        return;
    }
    archive_write_set_format_ustar(a);

    if (archive_write_open_filename(a, tar_file)) {
        std::cerr << "Error: " << archive_error_string(a) << std::endl;
        free(a, disk);
        return;
    }

    archive_read_disk_set_standard_lookup(disk);

    if (archive_read_disk_open(disk, src)) {
        std::cerr << "Error: " << archive_error_string(disk) <<  std::endl;
        free(a, disk);
        return;
    }

    while (true) {
        entry = archive_entry_new();
        status = archive_read_next_header2(disk, entry);

        if (status == ARCHIVE_EOF) break;
        
        if (status) {
            std::cerr << "Error: " << archive_error_string(disk) << std::endl;
            break;
        }

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

    free(a, disk);
    std::cout << "Tar file created successfully: " << tar_file << std::endl;
}

void compression_extract(const char *const tar, const char *const dest, const compression_type_t type)
{
    archive *a = archive_read_new();
    archive *disk = archive_write_disk_new();
    archive_entry *entry = nullptr;
    compression_type_t use_type = type; 
    size_t size;
    int64_t offset;

    int status = ARCHIVE_OK;
    const void *buff;
    char destination[FILENAME_MAX];

    
    if (use_type == UNKNOWN) {
        use_type = get_compression_type(tar);
        if (use_type == UNKNOWN) {
            std::cerr << "Error: Unknown compression type" << std::endl;
            return;
        }
    }

    if (dest == nullptr || strcmp(dest, ".") == 0) {
        getcwd(destination, sizeof(destination));
    } 
    else {
        // check for directory availability 
        if (!std::filesystem::is_directory(dest)) {
            std::cerr<<"Directory not found";
            return;
        }
        strncpy(destination, dest, FILENAME_MAX);
    }

    std::cout << "destination folder is: " << destination << std::endl;

    filter(a, use_type, READ);
    archive_read_support_format_tar(a);

    if (status = archive_read_open_filename(a, tar, BLOCK_SIZE)) {
        std::cerr<< "Error: " << archive_error_string(a);
        return;
    }

    while (true) {
        status = archive_read_next_header(a, &entry);
        if (status == ARCHIVE_EOF) {
            break;
        } 
        else if (status) {
            std::cerr<< "Error: " << archive_error_string(a);
            break;
        } 

        // extract
        if (archive_write_header(disk, entry)) {
            std::cerr<< "Error: " << archive_error_string(disk);
            break;
        }

        while (true) {
            status = archive_read_data_block(a, &buff, &size, &offset);
            if (status == ARCHIVE_EOF) break;
            else if (status) break;

            if (archive_write_data_block(disk, buff, size, offset)) {
                std::cerr<< "Error: " << archive_error_string(disk);
                break;
            }
        }

        if (archive_write_finish_entry(disk)) {
            std::cerr<< "Error: " << archive_error_string(disk);
            break;
        }
    }

    free(disk,a);
}
// close and free archive objects
static void free(archive *write, archive *read)
{
    std::cout << "Close archive objects:";

    if (write) {
        std::cout << " archive";
        archive_write_close(write);
        archive_write_free(write);
    }

    if (write) {
        std::cout << " disk";
        archive_read_close(read);
        archive_read_free(read);
    }

    std::cout << std::endl;
}

// autodetect compression type
static compression_type_t get_compression_type(const char *const file)
{
    std::filesystem::path path(file);
    
    if (path.extension() == ".gz") {
        return TAR_GZ;
    }
    else if (path.extension() == ".bz2") {
        return TAR_BZ2;
    }
    else if (path.extension() == ".xz") {
        return TAR_XZ;
    }
    else if (path.extension() == ".zst") {
        return TAR_ZST;
    }
    else if (path.extension() == ".lz4") {
        return TAR_LZ4;
    }
    else if (path.extension() == ".lzma") {
        return TAR_LZMA;
    }
    else if (path.extension() == ".zip") {
        return TAR_ZIP;
    }
    else if (path.extension() == ".rar") {
        return TAR_RAR;
    }
    else if (path.extension() == ".7z") {
        return TAR_7Z;
    }
    else {
        return UNKNOWN;
    }
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
