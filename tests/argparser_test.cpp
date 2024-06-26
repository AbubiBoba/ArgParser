#include <lib/argparser/ArgParser.hpp>
#include <gtest/gtest.h>
#include <sstream>


using namespace ArgumentParser;

std::vector<std::string> SplitString(const std::string& str) {
    std::istringstream iss(str);

    return {std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>()};
}


TEST(ArgParserTestSuite, EmptyTest) {
    ArgParser parser("My Empty Parser");

    ASSERT_TRUE(parser.Parse(SplitString("app")));
}


TEST(ArgParserTestSuite, StringTest) {
    ArgParser parser("My Parser");
    parser.AddStringArgument("param1");

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=value1")));
    ASSERT_TRUE(parser.GetValue<std::string>("param1").has_value());
    ASSERT_EQ(parser.GetValue<std::string>("param1").value(), "value1");
}


TEST(ArgParserTestSuite, ShortNameTest) {
    ArgParser parser("My Parser");
    parser.AddStringArgument('p', "param1");

    ASSERT_TRUE(parser.Parse(SplitString("app -p=value1")));
    ASSERT_TRUE(parser.GetValue<std::string>("param1").has_value());
    ASSERT_EQ(parser.GetValue<std::string>("param1").value(), "value1");
}


TEST(ArgParserTestSuite, DefaultTest) {
    ArgParser parser("My Parser");
    parser.AddStringArgument("param1").Default("value1");

    ASSERT_TRUE(parser.Parse(SplitString("app")));
    ASSERT_TRUE(parser.GetValue<std::string>("param1").has_value());
    ASSERT_EQ(parser.GetValue<std::string>("param1").value(), "value1");
}


TEST(ArgParserTestSuite, NoDefaultTest) {
    ArgParser parser("My Parser");
    parser.AddStringArgument("param1");

    ASSERT_FALSE(parser.Parse(SplitString("app")));
}


TEST(ArgParserTestSuite, StoreValueTest) {
    ArgParser parser("My Parser");
    std::string value;
    parser.AddStringArgument("param1").StoreValue(value);

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=value1")));
    ASSERT_EQ(value, "value1");
}


TEST(ArgParserTestSuite, MultiStringTest) {
    ArgParser parser("My Parser");
    std::string value;
    parser.AddStringArgument("param1").StoreValue(value);
    parser.AddStringArgument('a', "param2");

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=value1 --param2=value2")));
    ASSERT_TRUE(parser.GetValue<std::string>("param2").has_value());
    ASSERT_EQ(parser.GetValue<std::string>("param2").value(), "value2");
}


TEST(ArgParserTestSuite, IntTest) {
    ArgParser parser("My Parser");
    parser.AddIntArgument("param1");

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=100500")));
    ASSERT_EQ(parser.GetValue<int>("param1").value(), 100500);
}


TEST(ArgParserTestSuite, MultiValueTest) {
    ArgParser parser("My Parser");
    std::vector<int> int_values;
    parser.AddIntArgument('p', "param1").MultiValue().StoreValues(int_values);

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=1 --param1=2 --param1=3")));
    ASSERT_EQ(parser.GetValues<int>("param1").has_value(), true);
    ASSERT_EQ(parser.GetValues<int>("param1").value()[0], 1);
    ASSERT_EQ(int_values[1], 2);
    ASSERT_EQ(int_values[2], 3);
}


TEST(ArgParserTestSuite, MinCountMultiValueTest) {
    ArgParser parser("My Parser");
    std::vector<int> int_values;
    size_t MinArgsCount = 10;
    parser.AddIntArgument('p', "param1").MultiValue(MinArgsCount).StoreValues(int_values);

    ASSERT_FALSE(parser.Parse(SplitString("app --param1=1 --param1=2 --param1=3")));
}


TEST(ArgParserTestSuite, PositionalArgTest) {
    ArgParser parser("My Parser");
    std::vector<int> values;
    parser.AddIntArgument("Param1").MultiValue(1).Positional().StoreValues(values);

    ASSERT_TRUE(parser.Parse(SplitString("app 1 2 3 4 5")));
    ASSERT_EQ(values[0], 1);
    ASSERT_EQ(values[2], 3);
    ASSERT_EQ(values.size(), 5);
}

TEST(ArgParserTestSuite, FlagTest) {
    ArgParser parser("My Parser");
    parser.AddFlag('f', "flag1");

    ASSERT_TRUE(parser.Parse(SplitString("app --flag1")));
    ASSERT_TRUE(parser.GetValue<bool>("flag1").value_or(false));
}


TEST(ArgParserTestSuite, FlagsTest) {
    ArgParser parser("My Parser");
    bool flag3;
    parser.AddFlag('a', "flag1");
    parser.AddFlag('b', "flag2").Default(true);
    parser.AddFlag('c', "flag3").StoreValue(flag3);

    ASSERT_TRUE(parser.Parse(SplitString("app -ac")));

    ASSERT_TRUE(parser.GetValue<bool>("flag1").value_or(false));
    ASSERT_TRUE(parser.GetValue<bool>("flag2").has_value());
    ASSERT_TRUE(parser.GetValue<bool>("flag2").value());
    ASSERT_TRUE(flag3);
}


TEST(ArgParserTestSuite, HelpTest) {
    ArgParser parser("My Parser");
    parser.AddHelp('h', "help", "Some Description about program");

    ASSERT_TRUE(parser.Parse(SplitString("app --help")));
    ASSERT_TRUE(parser.Help());
}


TEST(ArgParserTestSuite, NegativePositionalTest) {
    ArgParser parser("My Parser");
    std::vector<int> values;
    parser.AddIntArgument("Param1").MultiValue(1).Positional().StoreValues(values);

    ASSERT_TRUE(parser.Parse(SplitString("app -1 -2 -3 -4 -5")));
    ASSERT_EQ(values[0], -1);
    ASSERT_EQ(values[2], -3);
    ASSERT_EQ(values.size(), 5);
}


TEST(ArgParserTestSuite, SplitterTest) {
    ArgParser parser("My Parser");
    std::vector<std::string> values;
    parser.AddIntArgument("param").Default(0);
    parser.AddStringArgument("pos").MultiValue(1).Positional().StoreValues(values);

    ASSERT_TRUE(parser.Parse(SplitString("app some splitter test -- --param=42")));

    ASSERT_TRUE(parser.GetValue<int>("param").has_value());
    ASSERT_EQ(parser.GetValue<int>("param").value(), 0);

    ASSERT_TRUE(parser.GetValues<std::string>("pos").has_value());
    ASSERT_EQ(parser.GetValues<std::string>("pos").value().size(), 4);
    ASSERT_EQ(parser.GetValues<std::string>("pos").value().back(), "--param=42");
}


TEST(ArgParserTestSuite, StrongPositionalTest) {
    ArgParser parser("My Parser");
    std::vector<int> values;
    parser.AddIntArgument('p', "param").MultiValue(1).Positional().StoreValues(values);

    ASSERT_TRUE(parser.Parse(SplitString("app -p -1 -p=-2 0 -0 +0 --param=+3 --param 4 -5 +3")));

    std::vector<int> answer = { -1, -2, 0, 0, 0, 3, 4, -5, 3 };
    ASSERT_EQ(values.size(), answer.size());
    for (int i = 0; i < values.size(); ++i) {
        ASSERT_EQ(values[i], answer[i]);
    }
}


TEST(ArgParserTestSuite, MyOwnFileTest) {
    ArgParser parser("My Parser");
    std::vector<int> values;
    parser.AddStringArgument("file").Positional();
    parser.AddStringArgument("input");

    ASSERT_TRUE(parser.Parse(SplitString("app --input=io.txt picture.png")));
    ASSERT_EQ(parser.GetValue<std::string>("file").has_value(), true);
    ASSERT_EQ(parser.GetValue<std::string>("file").value(), "picture.png");

    ASSERT_EQ(parser.GetValue<std::string>("input").has_value(), true);
    ASSERT_EQ(parser.GetValue<std::string>("input").value(), "io.txt");
}


TEST(ArgParserTestSuite, DefaultFlagTest) {
    ArgParser parser("My Parser");

    parser.AddFlag('f', "flag");

    ASSERT_TRUE(parser.Parse(SplitString("app")));
    ASSERT_EQ(parser.GetValue<bool>("flag").value_or(true), false);
}


TEST(ArgParserTestSuite, NoSuchArgTest) {
    ArgParser parser("My Parser");
    parser.AddIntArgument("param1");

    ASSERT_TRUE(parser.Parse(SplitString("app --param1=100500")));
    ASSERT_FALSE(parser.GetValue<double>("param1").has_value());
    ASSERT_FALSE(parser.GetValue<std::string>("param1").has_value());
    ASSERT_FALSE(parser.GetValue<int>("NOTparam1").has_value());
}


TEST(ArgParserTestSuite, ArchiveSampleTest) {
    ArgParser parser("My Parser");
    parser.AddFlag('x', "extract");
    parser.AddStringArgument('f', "file");

    parser.AddFlag('o', "open");
    parser.AddStringArgument('a', "archive");

    ASSERT_TRUE(parser.Parse(SplitString("app -xf=io.txt -oa arc.zip")));

    ASSERT_TRUE(parser.GetValue<bool>("extract").value_or(false));
    ASSERT_TRUE(parser.GetValue<std::string>("file").has_value());
    ASSERT_EQ(parser.GetValue<std::string>("file").value(), "io.txt");
    
    ASSERT_TRUE(parser.GetValue<bool>("open").value_or(false));
    ASSERT_TRUE(parser.GetValue<std::string>("archive").has_value());
    ASSERT_EQ(parser.GetValue<std::string>("archive").value(), "arc.zip");
}


TEST(ArgParserTestSuite, HelpStringTest) {
    ArgParser parser("My Parser");
    parser.AddHelp('h', "help", "Some Description about program");
    parser.AddStringArgument('i', "input", "File path for input file").MultiValue(1);
    parser.AddFlag('s', "flag1", "Use some logic").Default(true);
    parser.AddFlag('p', "flag2", "Use some logic");
    parser.AddIntArgument("numer", "Some Number");


    ASSERT_TRUE(parser.Parse(SplitString("app --help")));

    // ASSERT_EQ(
    //     parser.HelpDescription(),
    //     "My Parser\n"
    //     "Some Description about program\n"
    //     "\n"
    //     "-i,  --input=<string>,  File path for input file [repeated, min args = 1]\n"
    //     "-s,  --flag1,  Use some logic [default = true]\n"
    //     "-p,  --flag2,  Use some logic\n"
    //     "     --number=<int>,  Some Number\n"
    //     "\n"
    //     "-h, --help Display this help and exit\n"
    // );
}


TEST(ExternalInteractionsArgParserTestSuite, ExterlnalDoubleArgTest) {
    using namespace ArgumentData;

    class DoubleArg final : public Argument<double> {
    public:

        virtual ParseStatus ParseAndSave(std::string_view arg) override {
            std::stringstream ss;
            ss << arg;
            double value;
            ss >> value;
            if (ss.eof()) {
                was_parsed = true;
                storage.Save(value);
                return ParseStatus::kParsedSuccessfully;
            }
            return ParseStatus::kNotParsed;
        }

        std::string_view GetTypename () const override {
            return "double";
        }
    };

    ArgumentParser::ArgParser parser("Program");

    parser.AddArgument<DoubleArg>("billion", true);
    parser.AddArgument<DoubleArg>("pi", true);
    parser.AddArgument<DoubleArg>("e", true);

    ASSERT_TRUE(parser.Parse(SplitString("app --billion=1e9 --e=2.71 --pi 3.14")));

    ASSERT_EQ(parser.GetValue<double>("billion").value_or(0.0), 1e9);
    ASSERT_EQ(parser.GetValue<double>("e").value_or(0.0), 2.71);
    ASSERT_EQ(parser.GetValue<double>("pi").value_or(0.0), 3.14);
}


TEST(ExternalInteractionsArgParserTestSuite, CustomArgExternalPushTest) {
    using namespace ArgumentData;

    class SizedString {
    public:
        std::string value;
        size_t threshold = 1;
    };

    class CustomArg final : public Argument<SizedString> {
    public:
        virtual ParseStatus ParseAndSave(std::string_view arg) override {
            if (arg.size() > storage.GetValue().threshold) {
                return ParseStatus::kNotParsed;
            }
            Save(arg);
            was_parsed = true;
            return ParseStatus::kParsedSuccessfully;
        }
        CustomArg& SetThreshold(size_t threshold) {
            storage.GetValue().threshold = threshold;
            return *this;
        }
        void Save(std::string_view value) {
            storage.GetValue().value = value;
        }
    };

    ArgumentParser::ArgParser parser("parser");
    
    CustomArg* arg = new CustomArg;
    arg->Initialize("arg", "", true);
    arg->SetThreshold(5);
    parser.PushArgument(arg);

    ASSERT_TRUE(parser.Parse(SplitString("app --arg=world")));
    ASSERT_TRUE(parser.GetValue<SizedString>("arg").has_value());
    ASSERT_EQ(parser.GetValue<SizedString>("arg").value().value, "world");
}

