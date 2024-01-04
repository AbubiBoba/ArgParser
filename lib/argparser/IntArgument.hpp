#pragma once

namespace IntArgument {

using namespace Builder;
using namespace ArgumentData;

class IntArg : public Argument<int> {

    bool IsInt(std::string_view arg) const {
        bool is_int = std::isdigit(arg[0]) || arg[0] == '-';
        for (int i = 1; i < arg.size(); ++i) {
            if (!std::isdigit(arg[i]) || !is_int) {
                return false;
            }
        }
        return true;
    }

    ParseStatus ParseAndSave(std::string_view arg) override {

        if (arg.size() && IsInt(arg)) {
            int value;
            std::from_chars(arg.data(), arg.data() + arg.size(), value);
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
