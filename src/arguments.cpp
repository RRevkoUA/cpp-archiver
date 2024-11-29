#include "arguments.hpp"
#include "argparse/argparse.hpp"
#include "main.hpp"
#include "compression.hpp"

static void show_version();
static void compress_file(const char *const file, const char *const archive = nullptr);
static void extract_archive(const char *const archive, const char *const dst = nullptr);

static argparse::ArgumentParser program(PROJECT_NAME, PROJECT_VERSION);


void arg_configure()
{
    program.add_argument("-v", "--version")
        .help("show program version")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("-c", "--compress")
        .help("compress a file or directory")
        .default_value(false)
        .implicit_value(true)
        .nargs(1,2);

    program.add_argument("-e", "--extract")
        .help("extract to directory")
        .default_value(false)
        .implicit_value(true)
        .nargs(1,2);
}

void arg_parse(int argc, const char *const argv[])
{
    program.parse_args(argc, argv);

    if (program.is_used("-c") && program.is_used("-e"))
    {
        std::cerr << "Error: Cannot use both compress and extract options" << std::endl;
    }
    else if (program.is_used("-v"))
    {
        show_version();
    }
    else if (program.is_used("--compress"))
    {
        auto files = program.get<std::vector<std::string>>("--compress");
        const char *src = files[0].c_str();
        const char *archive = files.size() == 2 ? files[1].c_str() : nullptr;
        compress_file(src, archive);
    }
    else if (program.is_used("--extract"))
    {
        auto files = program.get<std::vector<std::string>>("--extract");
        const char *archive = files[0].c_str();
        const char *dst = files.size() == 2 ? files[1].c_str() : nullptr;
        extract_archive(archive, dst);
    }
    else
    {
        std::cout << program;
    }
}

static void show_version()
{
    std::cout << "Version: " << PROJECT_VERSION << std::endl;
}

static void compress_file(const char *const src, const char *const archive)
{
    compression_compress(src, archive);
}

static void extract_archive(const char *const archive, const char *const dst)
{
    compression_extract(archive, dst);
}