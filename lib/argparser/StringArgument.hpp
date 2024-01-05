#pragma once

#include <string>

namespace ArgumentParser {

using namespace Builder;
using namespace ArgumentData;

class StringArg final : public Argument <std::string> {

    ParseStatus ParseAndSave(std::string_view arg) override {

        was_parsed = true;
        if (is_multivalue) {
            storage.multi->push_back(std::string(arg));
        }
        else {
            *(storage.single) = std::string(arg);
        }

        return ParseStatus::kParsedSuccessfully;
    }
};

} // namespace StringArgument
