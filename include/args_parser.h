#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <map>
#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <charconv>
#include <exception>
#include <type_traits>
#include <algorithm>


namespace util {
    static const std::string TRUE{"true"};
    static const std::string FALSE{"false"};

    // a helper function to be used in concatenate function
    template<typename T>
    std::string to_string(T arg) {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>) return arg;
        else if constexpr (std::is_constructible_v<std::string, std::decay_t<T>>)
            return std::string{std::forward<T>(arg)};
        else if constexpr (std::is_same_v<char, std::decay_t<T>>) return {arg};
        else if constexpr (std::is_same_v<bool, std::decay_t<T>>) return arg ? TRUE : FALSE;
            // otherwise, all arithmetic values are supported by std::to_string
        else return std::to_string(std::forward<T>(arg));
    }

    // a function to concatenate string-ified version of different data types
    template<typename... Ts>
    std::string concatenate(Ts &&... args) {
        return (to_string(std::forward<Ts>(args)) + ...);
    }

    // throws only if status is false
    // no char* to std::string conversion unless status is actually false
    template<typename T=std::invalid_argument, typename ... Ts>
    void assert_statement(bool status, Ts... args) {
        if (!status) {
            throw T{concatenate(std::forward<Ts>(args)...)};
        }
    }

    // if input is not valid, it throws
    template<typename T>
    T from_string(const std::string &src) {
        if constexpr (std::is_same_v<std::string, std::decay_t<T>>) {
            return src;
        } else if constexpr (std::is_same_v<char, std::decay_t<T>>) {
            assert_statement(src.size() == 1,
                             "Can't convert [", src, "], size: [", src.size(), "] to char");
            return src.at(0);

        } else if constexpr (std::is_same_v<bool, std::decay_t<T>>) {
            assert_statement(src == TRUE || src == FALSE,
                             "Invalid value [", src, "] to parse to bool, expected values:[", TRUE, " / ", FALSE, "].");
            return src == TRUE;

        } else { // rest is all numerical types which std::from_chars can handle
            T value{};
            auto res = std::from_chars(src.data(), src.data() + src.size(), value);
            assert_statement(res.ec == std::errc() && res.ptr == src.data() + src.size(),
                             "Invalid string [", src, "] to convert to numeric type");
            return value;
        }
    }

    /* To strip the string (only from beginning and end) off any characters in @chars
     * - with default of being space chars
     * e.g. strip("   Hello World! ") -> "Hello World!"
     * */
    inline
    std::string strip(std::string_view source, std::string_view chars = " ") {
        auto first = source.find_first_not_of(chars);
        auto last = source.find_last_not_of(chars);
        return std::string{source.substr(first, last - first + 1)};
    }

    /*
     * To split a function based on a separator string
     * - if there are repeated occurrences of the separator, all are ignored
     * e.g. split("Hello123World", "123") -> ["Hello", "World"]
     * */
    inline
    std::vector<std::string> split(std::string_view source, const std::string_view sep) {
        // some quick sanity checks
        while (source.starts_with(sep)) { // as source might have multiple occurrences of sep at the beginning
            source = source.substr(sep.size());
        }

        std::vector<std::string> sub_strings{};
        while (!source.empty()) {
            // invariant: source is not empty and doesn't start with @sep
            const auto pos = source.find(sep);
            sub_strings.emplace_back(source.substr(0, pos));

            if (pos == std::string_view::npos) break;
            // @pos + @sep.size() can't exceed @source.size() as:
            // 1) @pos will either be @std::string_view::npos (handled previously) or
            // 2) @sep was actually found in @source
            source = source.substr(pos + sep.size());

            while (source.starts_with(sep)) { // as source might have multiple occurrences of sep at the beginning
                source = source.substr(sep.size());
            }
        }
        return sub_strings;
    }

    /*
     * To split a function based on a separator
     * - if there are repeated occurrences of the separator, all are ignored
     * e.g.
     * split("Hello! ") -> ["Hello!"]
     * split("Hello! This is a      test ") -> ["Hello!", "This", "is", "a", "test"]
     * */
    inline
    std::vector<std::string> split(std::string_view source, const char sep = ' ') {
        return split(source, to_string(sep));
    }

    template<typename ContainerT, typename ValueT>
    bool contains(const ContainerT &container, const ValueT &value) {
        return std::find(std::cbegin(container), std::cend(container), value) != std::cend(container);
    }
}

/*
 * A class to add command line arguments and parse them*/
class ArgsParser {
public:

    class ParsedArgs {
    public:
        /**
         * Initializes the instance with passed arg value pairs
         * @param arg_value_map: a map containing arg-value pairs of passed arguments
         */
        explicit ParsedArgs(std::map<std::string, std::string> arg_value_map);

        /**
         * A function used to retrieve value of an argument
         * @tparam T : the type of expected value, defaults to @std::string
         * @param arg : the arg whose value to be extracted
         * @return value of the @arg
         *
         * Throws @std::invalid_argument exception if arg is unknown or the value can't be converted to expected type
         */
        template<class T=std::string>
        T get(const std::string &arg) const noexcept(false);

        /**
         * A function used to retrieve value of an argument, wrapped in std::optional
         * @tparam T : the type of expected value, defaults to @std::string
         * @param arg : the arg whose value to be extracted
         * @return value of the @arg, wrapped in std::optional. Returns @std::nullopt
         *         if arg is unknown or the value can't be converted to expected type
         * Never throws, returning @std::nullopt is the way to represent unexpected scenario
         */
        template<class T=std::string>
        std::optional<T> get_opt(const std::string &arg) const noexcept;

        /**
         * A function to retrieve a list of values for an argument, where separator can be explicitly specified. .e.g.
         * - command line argument can be: --values=2,3,4,5 and
         * - and @get_list<int>("values") would return: @std::vector<int>{2,3,4,5}
         * Separator defaults to "," but it is not just limited to a single character separator, might as well be a string
         * @tparam T: type of value, can be all numerics, @bool, @char and @std::string (or a type which is used to construct an @std::string)
         * @param arg: the argument whose value to be extracted
         * @param sep: separator for passed values
         * @return: a @std::vector of values converted to type @T
         */
        template<typename T>
        std::vector<T> get_list(const std::string &arg, const std::string &sep = ",") const;

    private:
        std::map<std::string, std::string> _arg_value_map;
    };

    /**
     * To add an expected argument with some default value. Supported types for default values are:
     * - all numerics, bool, char and std::string (and any type which is used to construct std::string)
     * @tparam T: type of default value, defaults to @std::string
     * @param arg: the expected arg
     * @param default_value: default for the @arg's value. If it is not passed explicitly, this value is stored
     * @param description: A brief description of the argument
     * @return: @true if the @arg was successfully added, returns @false only when the same @arg was already added
     */
    template<class T = std::string>
    bool add_arg(std::string arg, T default_value, std::string description);

    /**
     * A deleted method to ensure that a list of values can't be used as a default value for an argument
     * Although, it is possible to add such an argument (of course, without default) and parse
     * - check @ParsedArgs::get_list method
     * It was deleted to keep the process of string-fying and storing the string-ified values clean as everything
     * under the hood is being stored in a @std::map<std::string, std::string>
     */
    template<class T>
    bool add_arg(std::string arg, std::vector<T> default_value, std::string description) = delete;

    /**
     * To add an argument
     * @param arg: the expected arg
     * @param description: A brief description of the argument
     * @param is_optional: @true if this argument is optional, otherwise @false. If it is @false and it is not
     *                     passed in command line arguments, then an exception will be raise while parsing the arguments
     * @return: @true if the @arg was successfully added, returns @false only when the same @arg was already added
     */
    bool add_arg(std::string arg, std::string description, bool is_optional = false);

    /**
     * To parse the command line arguments. The expected arguments is exactly what is passed to @main function
     * @param argc: count of arguments
     * @param argv: array of arguments
     * @return: an instance of @ParsedArgs which contains parsed argument value pairs
     */
    [[nodiscard]] ParsedArgs parse(int argc, char *argv[]);

private:

    static std::pair<std::string, std::string> validate_and_parse(std::string_view passed_arg) noexcept(false);

    void print_help() const;

    struct ArgsAttributes {
        bool is_optional;
        std::string default_value;
        std::string help;
    };
    std::string _app_path{};
    std::map<std::string, ArgsAttributes> _args;
};

template<class T>
[[nodiscard]] T ArgsParser::ParsedArgs::get(const std::string &arg) const noexcept(false) {
    auto itr = _arg_value_map.find(arg);
    util::assert_statement(itr != _arg_value_map.end(),
                           "Couldn't find [", arg, "] in arguments");
    return util::from_string<T>(itr->second);
}

template<class T>
std::optional<T> ArgsParser::ParsedArgs::get_opt(const std::string &arg) const noexcept {
    try {
        return get<T>(arg);
    } catch (const std::invalid_argument &exception) {
        return std::nullopt;
    }
}

template<class T>
bool ArgsParser::add_arg(std::string arg, T default_value, std::string description) {
    auto default_value_str = util::to_string(default_value);
    util::assert_statement(!default_value_str.empty(), "Default value is empty for [", arg, "]");
    return _args.emplace(std::move(arg),
                         ArgsAttributes{true, std::move(default_value_str), std::move(description)}).second;
}

inline
bool ArgsParser::add_arg(std::string arg, std::string description, bool is_optional) {
    return _args.emplace(std::move(arg), ArgsAttributes{is_optional, std::string{}, std::move(description)}).second;
}

inline
ArgsParser::ParsedArgs ArgsParser::parse(int argc, char *argv[]) {
    static constexpr std::array<const char *const, 3> help_args = {"help", "--help", "-h"};

    _app_path = argv[0];
    if (argc == 2 && util::contains(help_args, std::string{argv[1]})) {
        print_help();
        exit(0); // a rare case to call exit explicitly but seems like the right thing to do
    }

    std::map<std::string, std::string> arg_value_map{};

    for (int i = 1; i < argc; ++i) {
        auto arg_value_str = std::string(argv[i]);
        auto [arg, value] = validate_and_parse(arg_value_str);
        util::assert_statement(_args.contains(arg), "Unknown arg: [", arg, "]. Try --help");
        arg_value_map.emplace(std::move(arg), std::move(value));
    }

    // Now, taking care of args with default values which were not explicitly passed
    for (const auto &[arg, attributes]: _args) {
        if (attributes.is_optional && !arg_value_map.contains(arg)) {
            arg_value_map.emplace(arg, attributes.default_value);
        }

        // and ensuring that the mandatory arguments were indeed passed
        util::assert_statement(attributes.is_optional || arg_value_map.contains(arg),
                               "Mandatory argument [", arg, "] not passed in arguments. Try --help");
    }

    return ParsedArgs(std::move(arg_value_map));
}

inline
void ArgsParser::print_help() const {

    std::cout << "Following is the list of configured arguments for " << _app_path << ":\n";

    for (const auto &[arg, attributes]: _args) {
        std::cout << "--" << arg << "\n\t";
        std::cout << "Description: " << attributes.help << ", Optional: ["
                  << (attributes.is_optional ? util::TRUE : util::FALSE) << "]";

        if (attributes.is_optional && !attributes.default_value.empty()) {
            std::cout << ", Default value: [" << attributes.default_value << "]";
        }
        std::cout << "\n";
    }

    // now printing the help message
    std::cout << "--help\n\tDescription: To print this message\n";
    std::cout << std::endl;
}

inline
std::pair<std::string, std::string> ArgsParser::validate_and_parse(std::string_view passed_arg) noexcept(false) {
    // a handy lambda to be used for validating format
    const auto validate_format = [passed_arg](bool status) {
        util::assert_statement(status, "Unexpected format: [", passed_arg,
                               "], expected format is: [--arg=value]. Try --help");
    };

    validate_format(passed_arg.starts_with("--"));
    passed_arg = passed_arg.substr(2);

    validate_format(std::count(passed_arg.cbegin(), passed_arg.cend(), '=') == 1);
    const auto arg_value_pair = util::split(passed_arg, '=');
    validate_format(arg_value_pair.size() == 2);

    auto arg = util::strip(arg_value_pair.at(0));
    auto value = util::strip(arg_value_pair.at(1));
    validate_format(!arg.empty() && !value.empty());

    return std::make_pair(std::move(arg), std::move(value));
}

inline
ArgsParser::ParsedArgs::ParsedArgs(std::map<std::string, std::string> arg_value_map) : _arg_value_map(
        std::move(arg_value_map)) {}

template<typename T>
std::vector<T> ArgsParser::ParsedArgs::get_list(const std::string &arg, const std::string& sep) const {
    const auto strings = util::split(get(arg), sep);

    std::vector<T> parsed_values{};
    std::transform(strings.cbegin(), strings.cend(), std::back_inserter(parsed_values),
                   [](const std::string &v) { return util::from_string<T>(v); });
    return parsed_values;
}
