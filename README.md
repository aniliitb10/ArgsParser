<!-- TOC -->
  * [Introduction and sample usage](#introduction-and-sample-usage)
  * [Adding arguments](#adding-arguments)
  * [Parsing arguments](#parsing-arguments)
  * [Extracting values for arguments](#extracting-values-for-arguments)
  * [Description of added arguments](#description-of-added-arguments)
  * [Few examples](#few-examples)
  * [Installing the library](#installing-the-library)
<!-- TOC -->
## Introduction and sample usage
This is a header-only C++20 library for parsing command line arguments. Following is a sample usage:
```C++
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
```
Executing the app with command line arguments
```
$ ./cmake-build-debug/ArgsParser --ids=2,4,89 --log_path=new_log_file --timeout=7000
log_path is:[new_log_file]
timeout is:[7000]
Allowed ids:
2
4
89
```

## Adding arguments
1) `add_arg` method adds arguments. It has 2 overloads:

    i. `bool add_arg(std::string arg, T default_value, std::string description)` adds an argument with default value, hence making it implicitly optional. Supported types for default values are: all numerics, `bool`, `char` and `std::string` (and any type which is used to construct `std::string`)

    ii. `bool add_arg(std::string arg, std::string description, bool is_optional = false)` adds an argument where it can be explicitly marked optional. It is mandatory by default. 

2) If an argument is marked mandatory (`is_optional = false`), it must be passed as command line arguments. Otherwise, `parse` method will throw `std::invalid_argument` exception.
3) `add_arg` method returns `true` iff argument was added successfully. In case the argument was added previously, it returns `false`.

## Parsing arguments
`ArgsParser::parse` method returns an instance of `ArgsParser::ParsedArgs` which contains the parsed values. It stored the values in a `std::map<std::string, std::string>` and parses values when needed. Following are the cases when it throws `std::invalid_argument` exception:
1) When passed arguments are not in expected format. Expected format is: `--arg=value` or `--arg=value1,value2,value3` (It is not mandatory to use `,` as a separator for lists but same separator must be used in `get_list` method). So, following formats are **NOT acceptable**:
   
    i) `-arg=value` // it is supposed to start with '--'
    
    ii) `-arg==value` // It must contain just one '='

    iii) `--arg:value` // separator is '=' and not ':'

2) If a command line argument was not added but passed at runtime, `parse` method will raise `std::invalid_argument` exception
3) `bool` arguments must be passed as either `true` or `false`, e.g. `--enable=true` or `--raise=false`, any other formats like `T`, `True` etc. are NOT acceptable

## Extracting values for arguments
There are following 3 methods to extract an argument's value:
1) `T get(const std::string &arg) const noexcept(false)` extracts an argument's value and converts into type `T`. But if the argument is not found in configured set of arguments, or if the value can't be converted to type `T`, it will throw `std::invalid_argument` exception
2) `std::optional<T> get_opt(const std::string &arg) const noexcept` extracts an argument's value and converts into type `T`. But if the argument is not found in configured set of arguments, or if the value can't be converted to type `T`, then it returns `std::nullopt` and never throws an exception.
3) `std::vector<T> get_list(const std::string &arg, const std::string &sep = ",") const` extracts the values and converts to type `T` and returns them in an instance of `std::vector<T>`. Allowed types for `T` are: all numerics, `bool`, `char` and `std::string` (and any type which is used to construct `std::string`)

## Description of added arguments
Try `--help`, `help` or just `-h` to get the list of expected arguments and in this case, app will output the description and then, it will exit :
```bash
$ ./cmake-build-debug/ArgsParser --help
Following is the list of configured arguments for ./cmake-build-debug/ArgsParser:
--ids
        Description: Allowed ids, Optional: [false]
--log_path
        Description: Log file path for app, Optional: [false]
--timeout
        Description: Timeout for the app (seconds), Optional: [true], Default value: [60]
--help
        Description: To print this message
```

## Few examples
1) If an argument is NOT optional, then it must be passed while running the app. Otherwise, the `parse` method will raise exception
```bash
$ ./cmake-build-debug/ArgsParser
terminate called after throwing an instance of 'std::invalid_argument'
  what():  Mandatory argument [ids] not passed in arguments. Try --help
Aborted (core dumped)

```
2) Expected format is: `--arg=value`, blank space around `arg` and `value` is stripped, otherwise any other change in this format is not an acceptable and `parse` method will raise an `std::invalid_argument` exception

```bash
$ ./cmake-build-debug/ArgsParser --ids=2,4,89 --log_path=new_log_file --timeout=7000
log_path is:[new_log_file]
timeout is:[7000]
Allowed ids:
2
4
89
```
3) In the sample example, `timeout` is being extracted as an `int` (`parsed_args.get<int>("timeout")`). So if the value of `timeout` can't be converted to an `int`, exception will be thrown. Use `get_opt` method which returns `std::nullopt` instead of throwing an exception:
```bash
$ ./cmake-build-debug/ArgsParser --ids=2,4,89 --log_path=new_log_file --timeout=60m
log_path is:[new_log_file]
terminate called after throwing an instance of 'std::invalid_argument'
  what():  Invalid string [60m] to convert to numeric type
Aborted (core dumped)

```

## Installing the library
Since this is a header-only library, user just needs to copy the [header](https://github.com/aniliitb10/SimpleArgsParser/blob/master/include/args_parser.h) into its project and use it.