#include "arguments.hpp"
#include "argparse/argparse.hpp"
#include "main.hpp"

static void show_version();

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
}

void arg_parse(int argc, const char *const argv[])
{
    program.parse_args(argc, argv);

    if (program.is_used("--version"))
    {
        show_version();
        std::exit(0);
    }
}

static void show_version()
{
    std::cout << "Version: " << PROJECT_VERSION << std::endl;
}