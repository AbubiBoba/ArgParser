#include <numeric>
#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <lib/argparser/ArgParser.hpp>

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

    std::string_view GetTypename() const override {
        return "double";
    }
};

int main(int argc, char** argv) {

    ArgumentParser::ArgParser parser("Program");

    parser.AddArgument<DoubleArg>("billion", true);
    parser.AddArgument<DoubleArg>("pi", true);
    parser.AddArgument<DoubleArg>("e", true);

    if (!parser.Parse(std::vector<std::string_view>{ "app", "--billion=1e9", "--e=2.71", "--pi", "3.14" })) {
        std::cout << "Not parsed";
        return 0;
    }

    auto opt = parser.GetValue<double>("pi");
    std::cout << "billion = " << parser.GetValue<double>("billion").value_or(0.0) << std::endl;
    std::cout << "pi = " << parser.GetValue<double>("pi").value_or(0.0) << std::endl;
    std::cout << "e = " << parser.GetValue<double>("e").value_or(0.0) << std::endl;

    return 0;

}
