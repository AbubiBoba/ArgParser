#pragma once

#include <sstream>

namespace ArgumentParser {

using namespace ArgumentData;

class IntArg final : public Argument<int> {
    ParseStatus ParseAndSave(std::string_view arg) override {
        std::stringstream ss;
        ss << arg;
        int value;
        ss >> value;
        if (ss.eof()) {
            was_parsed = true;
            storage.Save(value);
            return ParseStatus::kParsedSuccessfully;
        }
        return ParseStatus::kNotParsed;
    }

    std::string_view GetTypename() const override {
        return "int";
    }
};

} // namespace IntArgument
