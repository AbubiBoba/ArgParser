#pragma once

namespace ArgumentParser {

using namespace Builder;
using namespace ArgumentData;

class BoolArg final : public Argument<bool> {
    ParseStatus ParseAndSave(std::string_view arg) override {

        if (arg.size()) {
            return ParseStatus::kNotParsed;
        }

        was_parsed = true;
        Save(true);

        return ParseStatus::kParsedSuccessfully;
    }

    void Save(const bool& value) override {
        if (multivalue_min_count.has_value()) {
            storage.multi->push_back(!default_value);
        }
        else {
            *storage.single = !*(storage.single);
        }
    }
};

} // namespace FlagArgument
