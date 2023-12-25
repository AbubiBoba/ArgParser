#pragma once

namespace IntArgument {

using namespace Builder;
using namespace ArgumentData;

class IntArg : public Argument<int> {

    bool IsInt(const std::string_view arg) const {
        bool is_int = std::isdigit(arg[0]) || arg[0] == '-';
        for (int i = 1; i < arg.size(); ++i) {
            is_int = is_int && std::isdigit(arg[i]);
        }
        return is_int;
    }

    ParseStatus Parse(const std::vector<std::string>& argv, int& iterator) override {

        const char kShortArgPrefix = '-';
        const std::string kLongArgPrefix = "--";
        const std::string& arg = argv[iterator];

        auto save_value = [](Argument<int>* data, int value) {
            data->was_parsed = true;
            if (data->is_multivalue) {
                data->storage.multi->push_back(value);
            }
            else {
                *(data->storage.single) = value;
            }
        };

        if (is_positional && IsInt({ arg.begin(), arg.end() })) {
            save_value(this, std::stoi(arg));
            return ParseStatus::kParsedSuccessfully;
        }

        bool take_next = false;
        if (has_nickname && (arg == kShortArgPrefix + std::string(1, nickname))) {
            take_next = true;
        }
        if (arg == kLongArgPrefix + fullname) {
            take_next = true;
        }

        if (take_next && iterator + 1 < argv.size() && IsInt(argv[iterator + 1])) {
            save_value(this, std::stoi(argv[++iterator]));
            return ParseStatus::kParsedSuccessfully;
        }
        else if (take_next) {
            return ParseStatus::kInvalidArguments;
        }

        std::string prefix = kLongArgPrefix + fullname + "=";
        if (arg.starts_with(prefix) && arg.size() > prefix.size() && IsInt({ arg.begin() + prefix.size(), arg.end() })) {
            save_value(this, std::stoi(std::string{ arg.begin() + prefix.size(), arg.end() }));
            return ParseStatus::kParsedSuccessfully;
        }

        if (!has_nickname) {
            return ParseStatus::kNotParsed;
        }

        prefix = kShortArgPrefix + nickname + "=";
        if (arg.starts_with(prefix) && arg.size() > prefix.size() && IsInt({ arg.begin() + prefix.size(), arg.end() })) {
            save_value(this, std::stoi(std::string{ arg.begin() + prefix.size(), arg.end() }));
            return ParseStatus::kParsedSuccessfully;
        }

        return ParseStatus::kNotParsed;
    }
};

} // namespace IntArgument
