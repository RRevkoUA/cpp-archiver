#include "arguments.hpp"
#include "argparse/argparse.hpp"
#include "main.hpp"
#include "compression.hpp"

static void compress_file(const char *const file, const char *const archive, compression_type_t type);
static void extract_archive(const char *const archive, const char *const dst, compression_type_t type);

static compression_type_t type_to_enum(const char *const type);
static std::string available_types();

static argparse::ArgumentParser program(PROJECT_NAME, PROJECT_VERSION);


void arg_configure()
{
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
    
    program.add_argument("-t", "--type")
        .help("Archive types " + available_types())
        .default_value("gz")
        .nargs(1);

}

void arg_parse(int argc, const char *const argv[])
{
    program.parse_args(argc, argv);

    if (program.is_used("-c") && program.is_used("-e"))
    {
        std::cerr << "Error: Cannot use both compress and extract options" << std::endl;
        throw std::runtime_error("Invalid arguments");
    }
    else if (program.is_used("--compress"))
    {
        auto files = program.get<std::vector<std::string>>("--compress");
        const char *src = files[0].c_str();
        const char *archive = files.size() == 2 ? files[1].c_str() : nullptr;
        auto type = program.get<std::string>("--type");
        compress_file(src, archive, type_to_enum(type.c_str()));
    }
    else if (program.is_used("--extract"))
    {
        auto files = program.get<std::vector<std::string>>("--extract");
        const char *archive = files[0].c_str();
        const char *dst = files.size() == 2 ? files[1].c_str() : nullptr;
        auto type = program.get<std::string>("--type");
        extract_archive(archive, dst, type_to_enum(type.c_str()));
    }
    else
    {
        std::cout << program;
    }
}

static void compress_file(const char *const src, const char *const archive, compression_type_t type)
{
    if (compression_compress(src, archive, type))
    {
        throw std::runtime_error("Error compressing file");
    };
}

static void extract_archive(const char *const archive, const char *const dst, compression_type_t type)
{
    if (compression_extract(archive, dst, type))
    {
        throw std::runtime_error("Error extracting archive");
    }
}

static compression_type_t type_to_enum(const char *const type)
{
    for (const auto &[key, value] : compression_map)
    {
        if (value == type)
        {
            return key;
        }
    }
    return UNKNOWN;
}

static std::string available_types()
{
    std::string types;
    for (const auto &[key, value] : compression_map)
    {
        types += value + " ";
    }
    return types;
}