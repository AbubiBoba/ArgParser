#pragma once

#include <string>

namespace ArgumentParser {

using namespace ArgumentData;

class StringArg final : public Argument <std::string> {
    ParseStatus ParseAndSave(std::string_view arg) override {

        was_parsed = true;
        storage.Save(std::string(arg));

        return ParseStatus::kParsedSuccessfully;
    }

    std::string_view GetTypename() const override {
        return "string";
    }
};

} // namespace StringArgument
