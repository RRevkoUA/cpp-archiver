#include "main.hpp"
#include "arguments.hpp"
#include "iostream"


int main(int argc, const char *const argv[])
{
    arg_configure();
    try {
        arg_parse(argc, argv);
    } catch (const std::exception &e) {
        std::cout << "An error occurred: ";
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}