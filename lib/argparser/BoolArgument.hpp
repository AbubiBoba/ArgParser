#pragma once

namespace BoolArgument {

using namespace Builder;
using namespace ArgumentData;

class BoolArg : public Argument<bool> {

    ParseStatus ParseAndSave(std::string_view arg) override {

        auto save_value = [](Argument<bool>* data) {
            data->was_parsed = true;
            if (data->is_multivalue) {
                data->storage.multi->push_back(!data->default_value);
            }
            else {
                *(data->storage.single) = !*(data->storage.single);
            }
        };

        if (arg.size()) {
            return ParseStatus::kNotParsed;
        }

        save_value(this);

        return ParseStatus::kParsedSuccessfully;
    }
};

} // namespace FlagArgument
