#pragma once


#include <string>
#include <string_view>
#include <optional>
#include <map>
#include <array>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <charconv>
#include <exception>
#include <string>
#include <type_traits>
#include <algorithm>
#include <string_view>

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
    template<typename T=std::runtime_error, typename ... Ts>
    void assert_statement(bool status, Ts... args) {
        if (!status) {
            throw T{concatenate(std::forward<Ts>(args)...)};
        }
    }

    // if input is not valid, it throws
    template<typename T = std::size_t>
    T from_string(const std::string &src) {
        if constexpr (std::is_same_v<bool, std::decay_t<T>>) {
            assert_statement<std::invalid_argument>(src == TRUE || src == FALSE,
                                                    "Invalid value [", src, "] to parse to bool, expected values:[",
                                                    TRUE, " / ", FALSE, "].");
            return src == TRUE;
        }

        T value{};
        auto res = std::from_chars(src.data(), src.data() + src.size(), value);
        assert_statement<std::invalid_argument>(res.ec == std::errc() && res.ptr == src.data() + src.size(),
                                                "Invalid string received [", src, "]");
        return value;
    }

    /* To strip the string (only from beginning and end) off the characters in @chars
     * - with default of being space chars
     * */
    inline
    std::string strip(std::string_view source, std::string_view chars = " ") {
        auto first = source.find_first_not_of(chars);
        auto last = source.find_last_not_of(chars);
        return std::string{source.substr(first, last - first + 1)};
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
        source = source.substr(source.find_first_not_of(sep));
        std::vector<std::string> strings{};

        while (!source.empty()) {
            auto end = source.find_first_of(sep);
            strings.emplace_back(source.substr(0, end));

            auto next_end = source.find_first_not_of(sep, end); // sep is at end
            if (next_end == std::string_view::npos) break;

            source = source.substr(next_end);
        }

        return strings;
    }

    template<typename ContainerT, typename ValueT>
    bool contains(const ContainerT &container, const ValueT &value) {
        return std::find(std::cbegin(container), std::cend(container), value) != std::cend(container);
    }
};

class SimpleArgsParser {
public:

    class ParsedArgs {
    public:
        explicit ParsedArgs(std::map<std::string, std::string> arg_value_map);

        template<class T=std::string>
        T get_may_throw(const std::string &field) noexcept(false);

        template<class T=std::string>
        std::optional<T> get(const std::string &field) noexcept;

    private:
        std::map<std::string, std::string> _arg_value_map;
    };

    template<class T = std::string>
    bool add_arg(std::string key, T default_value, std::string help_message);

    bool add_arg(std::string key, std::string help_message, bool is_optional = false);

    ParsedArgs parse(int argc, char **argv);

private:

    void print_help() const;

    struct ArgsAttributes {
        bool is_optional;
        std::string default_value;
        std::string help;
    };
    std::map<std::string, ArgsAttributes> _args;
};

template<class T>
T SimpleArgsParser::ParsedArgs::get_may_throw(const std::string &field) noexcept(false) {
    auto itr = _arg_value_map.find(field);
    util::assert_statement(itr == _arg_value_map.end(), "Couldn't find [", field, "] in arguments");
    return util::from_string<T>(itr->second);
}

template<class T>
std::optional<T> SimpleArgsParser::ParsedArgs::get(const std::string &field) noexcept {
    try {
        return get_may_throw(field);
    } catch (const std::invalid_argument &exception) {
        return std::nullopt;
    }
}

template<class T>
bool SimpleArgsParser::add_arg(std::string key, T default_value, std::string help_message) {
    auto default_value_str = util::to_string(default_value);
    util::assert_statement(!default_value_str.empty(), "Default value is empty for [", key, "]");
    return _args.try_emplace(std::move(key), true, std::move(default_value_str), std::move(help_message)).second;
}

inline
bool SimpleArgsParser::add_arg(std::string key, std::string help_message, bool is_optional) {
    return _args.try_emplace(std::move(key), is_optional, std::string{}, std::move(help_message)).second;
}

inline
SimpleArgsParser::ParsedArgs SimpleArgsParser::parse(int argc, char **argv) {
    static constexpr std::array<const char *const, 3> help_args = {"help", "--help", "-h"};

    if (argc == 2 && util::contains(help_args, std::string{argv[1]})) {
        print_help();
        exit(0); // a rare case to call exit explicitly but seems like the right thing to do
    }

    std::map<std::string, std::string> key_value_map{};

    for (int i = 1; i < argc; ++i) {
        auto arg_value_pair = std::string(argv[i]);
        auto split_args = util::split(arg_value_pair, '=');
        util::assert_statement(std::size(split_args) == 2, "There should be exactly one '=' in [", arg_value_pair,
                               "]. Try --help");

        auto key = util::strip(split_args.at(0));
        util::assert_statement(!key.empty(), "Key is empty in [", arg_value_pair, "]");
        util::assert_statement(_args.contains(key), "Unknown key: [", key, "]");

        auto value = util::strip(split_args.at(1));
        util::assert_statement(!key.empty(), "Value is empty in [", arg_value_pair, "]");

        key_value_map.emplace(std::move(key), std::move(value));
    }

    // Now, taking care of args with default values which were not explicitly passed
    for (const auto &[key, attributes]: _args) {
        if (attributes.is_optional && !key_value_map.contains(key)) {
            key_value_map.emplace(key, attributes.default_value);
        }

        util::assert_statement(!attributes.is_optional && key_value_map.contains(key),
                               "Mandatory argument [", key, "] not passed in arguments. Try --help");
    }

    return ParsedArgs(std::move(key_value_map));
}

inline
void SimpleArgsParser::print_help() const {

    if (_args.empty()) {
        std::cout << "There are no configured arguments" << std::endl;
        return;
    }

    std::cout << "Following is the configured arguments:\n";

    for (const auto &[arg, attributes]: _args) {
        std::cout << "Argument: [" << arg << "], is optional: " << "["
                  << (attributes.is_optional ? util::TRUE : util::FALSE) << "], ";
        if (attributes.is_optional) {
            std::cout << "Default value: [" << attributes.default_value << "], ";
        }
        std::cout << "Help message: " << attributes.help << "\n";
    }
    std::cout << std::endl;
}

inline
SimpleArgsParser::ParsedArgs::ParsedArgs(std::map<std::string, std::string> arg_value_map) : _arg_value_map(
        std::move(arg_value_map)) {}