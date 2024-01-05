#include <numeric>
#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <lib/argparser/ArgParser.hpp>

using namespace ArgumentData;
using namespace Builder;

class DoubleArg final : public Argument<double> {
public:

    virtual ParseStatus ParseAndSave(std::string_view arg) override {
        std::stringstream ss;
        ss << arg;
        double value;
        ss >> value;
        if (ss.eof()) {
            was_parsed = true;
            if (is_multivalue) {
                storage.multi->push_back(value);
            }
            else {
                *storage.single = value;
            }
            return ParseStatus::kParsedSuccessfully;
        }
        return ParseStatus::kNotParsed;
    }

};

int main(int argc, char** argv) {

    ArgumentParser::ArgParser parser("Program");

    parser.AddArgument<DoubleArg, double>("billion", true);
    parser.AddArgument<DoubleArg, double>("pi", true);
    parser.AddArgument<DoubleArg, double>("e", true);

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
