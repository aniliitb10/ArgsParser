#include <args_parser.h>
#include <gtest/gtest.h>
#include "../util.h"


struct SimpleArgsParserTest : public testing::Test{
    std::string DUMMY_APP_PATH{"host/prod/apps/test_app"}; 
};

TEST_F(SimpleArgsParserTest, DefaultArgumentTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("t", 5, "time interval"));
    EXPECT_FALSE(simple_args_parser.add_arg("t", 5, "time interval"));

    char* argv[] = {DUMMY_APP_PATH.data()};
    auto parsed_args = simple_args_parser.parse(1, argv);
    EXPECT_EQ(5, parsed_args.get<int>("t"));
    EXPECT_EQ(5, parsed_args.get_opt<int>("t").value());
}

TEST_F(SimpleArgsParserTest, DefaultArgumentExplicitTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("t", 5, "time interval"));

    std::string time_arg{"--t=60"};
    char* argv[] = {DUMMY_APP_PATH.data(), time_arg.data()};
    auto parsed_args = simple_args_parser.parse(2, argv);
    EXPECT_EQ(60, parsed_args.get<int>("t"));
    EXPECT_EQ(60, parsed_args.get_opt<int>("t").value());
}

TEST_F(SimpleArgsParserTest, MandatoryArgumentTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("t", "time interval", false));

    std::string time_arg{"--t=60"};
    char* argv[] = {DUMMY_APP_PATH.data(), time_arg.data()};
    auto parsed_args = simple_args_parser.parse(2, argv);
    EXPECT_EQ(60, parsed_args.get<int>("t"));
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-result"
TEST_F(SimpleArgsParserTest, MissingMandatoryArgumentTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("t", "time interval", false));

    char* argv[] = {DUMMY_APP_PATH.data()};
    EXPECT_EXCEPTION(simple_args_parser.parse(1, argv), std::invalid_argument, "Mandatory argument [t] not passed in arguments. Try --help");
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-result"
TEST_F(SimpleArgsParserTest, MissingOptionalArgumentTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("t", "time interval", true));

    char* argv[] = {DUMMY_APP_PATH.data()};
    EXPECT_NO_THROW(simple_args_parser.parse(1, argv));
}
#pragma clang diagnostic pop

TEST_F(SimpleArgsParserTest, ExtractingOptionalArgumentTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("t", "time interval", true));

    std::string time_arg{"--t=60"};
    char* argv[] = {DUMMY_APP_PATH.data(), time_arg.data()};
    auto parsed_args = simple_args_parser.parse(2, argv);
    EXPECT_EQ(60, parsed_args.get<int>("t"));
}

TEST_F(SimpleArgsParserTest, BooleanArgumentTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("e", "enable event", true));
    EXPECT_TRUE(simple_args_parser.add_arg("e_an", "enable another event", true));

    std::string event_arg{"--e=true"};
    std::string another_event_arg{"--e_an=false"};
    char* argv[] = {DUMMY_APP_PATH.data(), event_arg.data(), another_event_arg.data()};
    auto parsed_args = simple_args_parser.parse(3, argv);
    EXPECT_EQ(true, parsed_args.get<bool>("e"));
    EXPECT_EQ(false, parsed_args.get<bool>("e_an"));
}

TEST_F(SimpleArgsParserTest, DoubleArgumentTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("value", "testing value", true));
    EXPECT_TRUE(simple_args_parser.add_arg("n_value", "testing value", true));

    std::string value_arg{"--value=3.14"};
    std::string n_value_arg{"--n_value=-6.023"};
    char* argv[] = {DUMMY_APP_PATH.data(), value_arg.data(), n_value_arg.data()};
    auto parsed_args = simple_args_parser.parse(3, argv);
    EXPECT_DOUBLE_EQ(3.14, parsed_args.get<double>("value"));
    EXPECT_DOUBLE_EQ(-6.023, parsed_args.get<double>("n_value"));
}

TEST_F(SimpleArgsParserTest, IntArgumentTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("v", "value", true));
    EXPECT_TRUE(simple_args_parser.add_arg("nv", "negative value", true));

    std::string v_arg{"--v=97"};
    std::string nv_arg{"--nv=-585666"};
    char* argv[] = {DUMMY_APP_PATH.data(), v_arg.data(), nv_arg.data()};
    auto parsed_args = simple_args_parser.parse(3, argv);
    EXPECT_EQ(97, parsed_args.get<int>("v"));
    EXPECT_EQ(-585666, parsed_args.get<int>("nv"));
}

TEST_F(SimpleArgsParserTest, StrArgumentTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("v", "value", true));
    EXPECT_TRUE(simple_args_parser.add_arg("nv", "not that value", true));

    std::string v_arg{"--v=testing"};
    std::string nv_arg{"--nv=not testing"};
    char* argv[] = {DUMMY_APP_PATH.data(), v_arg.data(), nv_arg.data()};
    auto parsed_args = simple_args_parser.parse(3, argv);
    EXPECT_EQ("testing", parsed_args.get<std::string>("v"));
    EXPECT_EQ("not testing", parsed_args.get<std::string>("nv"));
}

TEST_F(SimpleArgsParserTest, ListArgumentTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("int_v", "list of int values", false));
    EXPECT_TRUE(simple_args_parser.add_arg("double_v", "list of double values", false));
    EXPECT_TRUE(simple_args_parser.add_arg("nv", "a negative int", true));

    std::string int_v_arg{"--int_v=2,3,4,0,"};
    std::vector<int> expected_ints{2,3,4,0};

    std::string double_v_arg{"--double_v=2.6|3.14|4.4489"}; // a different separator
    std::vector<double> expected_doubles{2.6,3.14,4.4489};

    std::string nv_arg{"--nv=-97"};
    char* argv[] = {DUMMY_APP_PATH.data(), int_v_arg.data(), double_v_arg.data(), nv_arg.data()};
    auto parsed_args = simple_args_parser.parse(4, argv);
    EXPECT_EQ(expected_ints, parsed_args.get_list<int>("int_v"));
    EXPECT_EQ(-97, parsed_args.get<int>("nv"));

    // comparing vectors of doubles
    const auto parsed_doubles = parsed_args.get_list<double>("double_v", "|");
    EXPECT_EQ(expected_doubles.size(), parsed_doubles.size());
    for (std::size_t i = 0; i < expected_doubles.size(); ++i) {
        EXPECT_DOUBLE_EQ(expected_doubles.at(i), parsed_doubles.at(i));
    }

}

TEST_F(SimpleArgsParserTest, GetOptTest) {
    ArgsParser simple_args_parser{};
    EXPECT_TRUE(simple_args_parser.add_arg("e", "enable event", true));
    EXPECT_TRUE(simple_args_parser.add_arg("d", "example double value", true));

    std::string event_arg{"--e=true"};
    std::string double_arg{"--d=4.325"};
    char* argv[] = {DUMMY_APP_PATH.data(), event_arg.data(), double_arg.data()};
    auto parsed_args = simple_args_parser.parse(3, argv);
    EXPECT_EQ(true, parsed_args.get_opt<bool>("e").value());
    EXPECT_DOUBLE_EQ(4.325, parsed_args.get_opt<double>("d").value());

    // error scenarios
    EXPECT_EQ(std::nullopt, parsed_args.get_opt<bool>("unknown arg"));
    EXPECT_EQ(std::nullopt, parsed_args.get_opt<bool>("d"));
}

TEST_F(SimpleArgsParserTest, InvalidFormatTest) {
    ArgsParser simple_args_parser{};

    std::string double_arg{"-d=4.325"};
    char* argv1 [] = {DUMMY_APP_PATH.data(), double_arg.data()};
    EXPECT_EXCEPTION(simple_args_parser.parse(2, argv1), std::invalid_argument,
                     "Unexpected format: [-d=4.325], expected format is: [--arg=value]. Try --help");

    std::string without_hyphen_arg{"d=4.325"};
    char* argv2[] = {DUMMY_APP_PATH.data(), without_hyphen_arg.data()};
    EXPECT_EXCEPTION(simple_args_parser.parse(2, argv2), std::invalid_argument,
                     "Unexpected format: [d=4.325], expected format is: [--arg=value]. Try --help");

    std::string multiple_sep_arg{"d==4.325"};
    char* argv3[] = {DUMMY_APP_PATH.data(), multiple_sep_arg.data()};
    EXPECT_EXCEPTION(simple_args_parser.parse(2, argv3), std::invalid_argument,
                     "Unexpected format: [d==4.325], expected format is: [--arg=value]. Try --help");

    std::string no_sep_arg{"d4.325"};
    char* argv4[] = {DUMMY_APP_PATH.data(), no_sep_arg.data()};
    EXPECT_EXCEPTION(simple_args_parser.parse(2, argv4), std::invalid_argument,
                     "Unexpected format: [d4.325], expected format is: [--arg=value]. Try --help");
}
