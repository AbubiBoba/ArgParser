#pragma once

#include <string>

namespace StringArgument {

using namespace Builder;
using namespace ArgumentData;

class StringArg : public Argument <std::string> {

    ParseStatus ParseAndSave(std::string_view arg) override {

        auto save_value = [](Argument<std::string>* data, std::string value) {
            data->was_parsed = true;
            if (data->is_multivalue) {
                data->storage.multi->push_back(value);
            }
            else {
                *(data->storage.single) = value;
            }
        };

        save_value(this, std::string(arg));

        return ParseStatus::kParsedSuccessfully;
    }
};

} // namespace StringArgument
