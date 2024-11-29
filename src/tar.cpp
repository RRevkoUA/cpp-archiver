#include <archive.h>
#include <archive_entry.h>
#include <string.h>
#include <fcntl.h>
#include <filesystem>
#include <iostream>

#define BUFFER_SIZE (16 * 1024)

static void compress_free(archive *a, archive *disk);

void tar_compress(const char *const src, const char *const tar)
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
        const char *dir_name = strrchr(src, int('/'));
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
        tar_file[FILENAME_MAX - 1] = '\0';
    }

    std::cout << "tar_file: " << tar_file << std::endl;

    if (!a || !disk) {
        std::cerr << "Error: Could not create archive structures" << std::endl;
        compress_free(a, disk);
        return;
    }

    // TODO :: Change by type of compression
    archive_write_add_filter_gzip(a);
    archive_write_set_format_ustar(a);

    if (archive_write_open_filename(a, tar_file)) {
        std::cerr << "Error: " << archive_error_string(a) << std::endl;
        compress_free(a, disk);
        return;
    }

    archive_read_disk_set_standard_lookup(disk);

    if (archive_read_disk_open(disk, src)) {
        std::cerr << "Error: " << archive_error_string(disk) <<  std::endl;
        compress_free(a, disk);
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
        status = archive_write_header(a, entry);

        if (status) {
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

    compress_free(a, disk);
    std::cout << "Tar file created successfully: " << tar_file << std::endl;
}

// close and free archive objects
static void compress_free(archive *a, archive *disk)
{
    std::cout << "Close archive objects:";

    if (a) {
        std::cout << " archive";
        archive_write_close(a);
        archive_write_free(a);
    }

    if (disk) {
        std::cout << " disk";
        archive_read_close(disk);
        archive_read_free(disk);
    }

    std::cout << std::endl;
}
