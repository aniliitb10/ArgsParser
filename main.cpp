#include <args_parser.h>
#include <iostream>

int main(int argc, char *argv[]) {
    ArgsParser args_parser{}; // to create an instance

    // call add_arg method to add an argument and make it mandatory
    // (by setting is_optional=false)
    args_parser.add_arg("log_path", "Log file path for app", false);

    // call an overload and pass default value
    // (hence, implicitly, make it an optional argument)
    args_parser.add_arg("timeout", 60, "Timeout for the app (seconds)");

    // add an argument, whose value is expected to be a list of values.
    // Nothing different here because of that, just add as usual
    args_parser.add_arg("ids", "Allowed ids");

    // done adding the arguments, let's parse the command line arguments
    const auto parsed_args = args_parser.parse(argc, argv);

    // Now, let's extract the stored values
    std::cout << "log_path is:[" << parsed_args.get("log_path") << "]\n";
    std::cout << "timeout is:[" << parsed_args.get<int>("timeout") << "]\n";
    std::cout << "Allowed ids:\n";

    // notice the different function call to get the list (get_list instead of get)
    for (auto id: parsed_args.get_list<int>("ids")) {
        std::cout << id << "\n";
    }

    return 0;
}