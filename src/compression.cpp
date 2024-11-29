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

void compression_compress(const char *const src, const char *const tar, const compression_type_t type)
{
    archive *a = archive_write_new();
    archive *disk = archive_read_disk_new();
    archive_entry *entry = nullptr;
   
    int status = ARCHIVE_OK;
    int fd = 0;
    char buff[BUFFER_SIZE];
    ssize_t len;

    if (!src || strlen(src) == 0) {
        std::cerr << "Error: Invalid source directory" << std::endl;
        return;
    }

    char tar_file[FILENAME_MAX];

    if (tar == nullptr || strcmp(tar, ".") == 0) {
        char cwd[FILENAME_MAX];
        const char *dir_name = strrchr(src, int('/')); // TODO :: check for irrationality of next 6 lines
        if (dir_name == nullptr) {
            dir_name = (char *)src;
        }
        else {
            dir_name++;
        }
        if (!getcwd(cwd, sizeof(cwd))) {
            std::cerr << "Error: Could not get current directory" << std::endl;
            return;
        }
        std::filesystem::path path = (src[0] == '.') ? cwd : src;
        snprintf(tar_file, sizeof(tar_file), "%s/%s.tar.gz", cwd, path.filename().c_str()); // TODO :: Change tar.gz to type of compression
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

    // TODO :: Change by type of compression
    archive_write_add_filter_gzip(a);
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

    int status = ARCHIVE_OK;
    const void *buff;
    size_t size;
    int64_t offset;

    char destination[FILENAME_MAX];
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

    archive_read_support_filter_gzip(a);
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
            else if (status) {
                
                break;
            }

            if (archive_write_data_block(disk, buff, size, offset))
            {
                std::cerr<< "Error: " << archive_error_string(disk);
                break;
            }
        }

        if (archive_write_finish_entry(disk))
        {
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