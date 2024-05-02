#include <numeric>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <lib/argparser/ArgParser.hpp>


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

int main(int argc, char** argv) {

    ArgumentParser::ArgParser parser("parser");

    CustomArg* arg = new CustomArg;
    arg->Initialize("arg", "", true);
    arg->SetThreshold(5);
    parser.PushArgument(arg);

    if (!parser.Parse(std::vector<std::string_view>{ "app", "--arg=world" })) {
        std::cout << "Not parsed";
        return 0;
    }

    auto opt = parser.GetValue<SizedString>("arg");
    if (opt) {
        std::cout << "arg = " << opt.value().value;
    }

    return 0;

}
