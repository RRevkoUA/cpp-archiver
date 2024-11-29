#include "arguments.hpp"
#include "argparse/argparse.hpp"
#include "main.hpp"
#include "tar.hpp"

static void show_version();
static void compress_file(const char *const file, const char *const archive = nullptr);

static argparse::ArgumentParser program(PROJECT_NAME, PROJECT_VERSION);


void arg_configure()
{
    program.add_argument("-v", "--version")
        .help("show program version")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("-h", "--help")
        .help("show help message")
        .default_value(false)
        .implicit_value(true);
        
    program.add_argument("-c", "--compress")
        .help("compress a file or directory")
        .default_value(false)
        .implicit_value(true)
        .nargs(1,2);
}

void arg_parse(int argc, const char *const argv[])
{
    program.parse_args(argc, argv);

    if (program.is_used("--version"))
    {
        show_version();
        std::exit(0);
    }
    else if (program.is_used("--compress"))
    {
        auto files = program.get<std::vector<std::string>>("--compress");
        const char *dir = files[0].c_str();
        const char *archive = files.size() == 2 ? files[1].c_str() : nullptr;
        compress_file(dir, archive);
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

static void compress_file(const char *const file, const char *const archive)
{
    tar_compress(file, archive);
}