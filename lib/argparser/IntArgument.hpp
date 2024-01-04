#pragma once

#include <sstream>

namespace IntArgument {

using namespace Builder;
using namespace ArgumentData;

class IntArg : public Argument<int> {

    ParseStatus ParseAndSave(std::string_view arg) override {
        std::stringstream ss;
        ss << arg;
        int value;
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

} // namespace IntArgument
