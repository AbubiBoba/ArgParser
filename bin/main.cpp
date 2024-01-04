#include <numeric>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <lib/argparser/ArgParser.hpp>


using namespace ArgumentData;
using namespace Builder;

class SizedString {
public:
    std::string value;
    size_t threshold = 1;
};

class CustomArg : public Argument<SizedString> {
public:

    virtual ParseStatus ParseAndSave(std::string_view arg) override {
        if (arg.size() > storage.single->threshold) {
            return ParseStatus::kNotParsed;
        }
        was_parsed = true;
        storage.single->value = arg;
        return ParseStatus::kParsedSuccessfully;
    }

};

class CustomBuilder : public IBuilder {
public:
    ~CustomBuilder() override {
        delete product;
    }

    CustomBuilder(const std::string& fullname) {
        Reset();
        product->fullname = fullname;
        product->has_param = true;
        product->storage.single = new SizedString;
    }

    CustomBuilder& SetThreshold(size_t threshold) {
        product->storage.single->threshold = threshold;
        return *this;
    }

    [[nodiscard]]
    ArgData* Build() override {
        ArgData* to_return = product;
        Reset();
        return to_return;
    }

    virtual const std::string& GetArgumentName() const override {
        return product->fullname;
    }
private:
    CustomArg* product = nullptr;

    void Reset() {
        product = new CustomArg{};
    }
};


int main(int argc, char** argv) {

    ArgumentParser::ArgParser parser("Program");

    /*  Variant I  */
    CustomBuilder builder("arg");
    builder.SetThreshold(5);
    parser.PushArgument(builder.Build());

    /*  Variant II  */
    /* parser.PushBuilder(new CustomBuilder("arg")).SetThreshold(5); */

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
