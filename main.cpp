#include <simple_args_parser.h>
#include <iostream>

int main(int argc, char** argv) {
    SimpleArgsParser args_parser{};
    args_parser.add_arg("log_file", "the full path of log file", false);
    args_parser.parse(argc, argv);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
