#pragma once

#include "ArgumentData.hpp"
#include "Builder.hpp"

#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>


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

namespace ArgumentParser {

using namespace Builder;
using namespace ArgumentData;

//using namespace StringArgument;
//using namespace FlagArgument;


class ArgParser {
public:
    ArgParser(const std::string& id);
    ArgParser(const ArgParser& other) = delete;
    ArgParser& operator=(const ArgParser& other) = delete;
    ~ArgParser();

    bool Parse(int argc, char** argv);
    bool Parse(std::vector<std::string> argv);

    void Build();
    bool IsValid() const;

    template<class ArgT, typename T>
        requires(std::is_base_of<Argument<T>, ArgT>::value)
    ArgBuilder<ArgT, T>& AddArgument(const std::string& fullname, const std::string& description = "", bool take_param = true) {
        ArgBuilder<ArgT, T>* builder = new ArgBuilder<ArgT, T>(fullname, description, take_param);
        return PushBuilder(builder);
    }
    template<class ArgT, typename T>
        requires(std::is_base_of<Argument<T>, ArgT>::value)
    ArgBuilder<ArgT, T>& AddArgument(char nickname, const std::string& fullname, const std::string& description = "", bool take_param = true) {
        return AddArgument<ArgT, T>(fullname, description, take_param).AddNickname(nickname);
    }

    template<class TBuilder>
        requires(std::is_base_of<IBuilder, TBuilder>::value)
    TBuilder& PushBuilder(TBuilder* builder) {
        builders.push_back(builder);
        return *builder;
    }

    template<class TArgData>
        requires(std::is_base_of<ArgData, TArgData>::value)
    void PushArgument(TArgData* arg_ptr) {
        arg_data[arg_ptr->fullname] = arg_ptr;
        if (arg_ptr->is_positional) {
            positional.push_back(arg_ptr);
        }
    }

    template<typename T>
    std::optional<T> GetValue(const std::string& name) {
        Argument<T>* p_arg = GetArgument<T>(name);
        if (!p_arg || p_arg->is_multivalue) {
            return {};
        }
        return *(p_arg->storage.single);
    }

    template<typename T>
    std::optional<std::vector<T>> GetValues(const std::string& name) {
        Argument<T>* p_arg = GetArgument<T>(name);
        if (!p_arg || !p_arg->is_multivalue) {
            return {};
        }
        return *(p_arg->storage.multi);
    }

    // Built-in types
    ArgBuilder<IntArgument::IntArg, int>& AddIntArgument(const std::string& fullname, const std::string& description = "") { return AddArgument<IntArgument::IntArg, int>(fullname, description); }
    ArgBuilder<IntArgument::IntArg, int>& AddIntArgument(char nickname, const std::string& fullname, const std::string& description = "") { return AddIntArgument(fullname, description).AddNickname(nickname); }

private:

    template<typename T>
    Argument<T>* GetArgument(const std::string& name) {
        auto iterator = arg_data.find(name);
        if (iterator == arg_data.end()) {
            return nullptr;
        }
        Argument<T>* p_arg = dynamic_cast<Argument<T>*>(arg_data[name]);
        return p_arg;
    }

    std::string name = "";
    std::map<std::string, ArgData*> arg_data;
    std::vector<ArgData*> positional;
    std::vector<IBuilder*> builders;
};

} // namespace ArgumentParser
