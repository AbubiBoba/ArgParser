#pragma once

#include <sstream>

namespace ArgumentParser {

using namespace Builder;
using namespace ArgumentData;

class IntArg final : public Argument<int> {

    ParseStatus ParseAndSave(std::string_view arg) override {
        std::stringstream ss;
        ss << arg;
        int value;
        ss >> value;
        if (ss.eof()) {
            was_parsed = true;
            if (multivalue_min_count.has_value()) {
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

} // namespace IntArgument
