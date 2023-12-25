#pragma once

namespace BoolArgument {

using namespace Builder;
using namespace ArgumentData;

class BoolArg : public Argument<bool> {

    ParseStatus Parse(const std::vector<std::string>& argv, int& iterator) override {

        const char kShortArgPrefix = '-';
        const std::string kLongArgPrefix = "--";

        const std::string& arg = argv[iterator];
        bool partly_parsed = false;

        if ((has_nickname && (arg == kShortArgPrefix + std::string(1, nickname))) || (arg == kLongArgPrefix + fullname)) {
            was_parsed = true;
            *(storage.single) = !*(storage.single);
            return ParseStatus::kParsedSuccessfully;
        }

        if (has_nickname && arg.starts_with(kShortArgPrefix) && arg.find(nickname) != std::string::npos) {
            was_parsed = true;
            *(storage.single) = !*(storage.single);
            partly_parsed = true;
        }

        return partly_parsed ? ParseStatus::kPartlyParsed : ParseStatus::kNotParsed;
    }
};

} // namespace FlagArgument
