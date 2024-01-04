#pragma once

namespace IntArgument {

using namespace Builder;
using namespace ArgumentData;

class IntArg : public Argument<int> {

    bool IsInt(std::string_view arg) const {
        bool is_int = std::isdigit(arg[0]) || arg[0] == '-';
        for (int i = 1; i < arg.size(); ++i) {
            is_int = is_int && std::isdigit(arg[i]);
        }
        return is_int;
    }

    ParseStatus ParseAndSave(std::string_view arg) override {

        auto save_value = [](Argument<int>* data, int value) {
            data->was_parsed = true;
            if (data->is_multivalue) {
                data->storage.multi->push_back(value);
            }
            else {
                *(data->storage.single) = value;
            }
        };

        if (arg.size() && IsInt({ arg.begin(), arg.end() })) {
            int value;
            std::from_chars(arg.data(), arg.data() + arg.size(), value);
            save_value(this, value);
            return ParseStatus::kParsedSuccessfully;
        }
        return ParseStatus::kNotParsed;
    }

};

} // namespace IntArgument
