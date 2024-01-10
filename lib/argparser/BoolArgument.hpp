#pragma once

namespace ArgumentParser {

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

    void Save(bool value) {
        if (multivalue_min_count.has_value()) {
            storage.Save(value);
        }
        else {
            storage.Save(!storage.GetValue());
        }
    }

    std::string_view GetTypename() const override {
        return "";
    }
};

} // namespace FlagArgument
