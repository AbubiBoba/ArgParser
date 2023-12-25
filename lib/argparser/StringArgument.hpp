#pragma once

#include <string>

namespace StringArgument {

using namespace Builder;
using namespace ArgumentData;

class StringArg : public Argument <std::string> {

    ParseStatus Parse(const std::vector<std::string>& argv, int& iterator) override {

        const char kShortArgPrefix = '-';
        const std::string kLongArgPrefix = "--";
        const std::string& arg = argv[iterator];

        auto save_value = [](Argument<std::string>* data, std::string value) {
            data->was_parsed = true;
            if (data->is_multivalue) {
                data->storage.multi->push_back(value);
            }
            else {
                *(data->storage.single) = value;
            }
        };

        if (is_positional) {
            save_value(this, arg);
            return ParseStatus::kParsedSuccessfully;
        }

        bool take_next = false;
        if (has_nickname && (arg == kShortArgPrefix + std::string(1, nickname))) {
            take_next = true;
        }
        if (arg == kLongArgPrefix + fullname) {
            take_next = true;
        }

        if (take_next && iterator + 1 < argv.size()) {
            save_value(this, argv[++iterator]);
            return ParseStatus::kParsedSuccessfully;
        }
        else if (take_next) {
            return ParseStatus::kInvalidArguments;
        }

        std::string prefix = kLongArgPrefix + fullname + "=";
        if (arg.starts_with(prefix) && arg.size() > prefix.size()) {
            save_value(this, std::string{ arg.begin() + prefix.size(), arg.end() });
            return ParseStatus::kParsedSuccessfully;
        }

        if (!has_nickname) {
            return ParseStatus::kNotParsed;
        }

        prefix = std::string(1, kShortArgPrefix) + std::string(1, nickname) + "=";
        if (arg.starts_with(prefix) && arg.size() > prefix.size()) {
            save_value(this, std::string{ arg.begin() + prefix.size(), arg.end() });
            return ParseStatus::kParsedSuccessfully;
        }

        return ParseStatus::kNotParsed;
    }
};

} // namespace StringArgument
