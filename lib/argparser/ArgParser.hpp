#pragma once

#include "ArgumentData.hpp"
#include "Builder.hpp"
#include "BoolArgument.hpp"
#include "IntArgument.hpp"
#include "StringArgument.hpp"

#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace ArgumentParser {

using namespace Builder;
using namespace ArgumentData;

class ArgParser {
public:
    ArgParser(std::string_view id);
    ArgParser(const ArgParser& other) = delete;
    ArgParser& operator=(const ArgParser& other) = delete;
    ~ArgParser();

    bool Parse(int argc, char** argv);
    bool Parse(const std::vector<std::string>& argv);
    bool Parse(const std::vector<std::string_view>& argv);

    template<class ArgT, typename T>
        requires(std::is_base_of<Argument<T>, ArgT>::value)
    ArgBuilder<ArgT, T>& AddArgument(const std::string& fullname, bool take_param, const std::string& description = "") {
        ArgBuilder<ArgT, T>* builder = new ArgBuilder<ArgT, T>(fullname, description, take_param);
        return PushBuilder(builder);
    }
    template<class ArgT, typename T>
        requires(std::is_base_of<Argument<T>, ArgT>::value)
    ArgBuilder<ArgT, T>& AddArgument(char nickname, const std::string& fullname, bool take_param, const std::string& description = "") {
        return AddArgument<ArgT, T>(fullname, description, take_param).AddNickname(nickname);
    }

    template<class TBuilder>
        requires(std::is_base_of_v<IArgumentBuilder, TBuilder>)
    TBuilder& PushBuilder(TBuilder* builder) {
        builders.push_back(builder);
        return *builder;
    }

    void PushArgument(ArgData* arg_ptr);

    template<typename T>
    std::optional<T> GetValue(std::string_view name) {
        Argument<T>* p_arg = GetArgument<T>(name);
        if (!p_arg || p_arg->multivalue_min_count.has_value()) {
            return {};
        }
        return *(p_arg->storage.single);
    }

    template<typename T>
    std::optional<std::vector<T>> GetValues(std::string_view name) {
        Argument<T>* p_arg = GetArgument<T>(name);
        if (!p_arg || !p_arg->multivalue_min_count.has_value()) {
            return {};
        }
        return *(p_arg->storage.multi);
    }

    // Built-in types
    ArgBuilder<IntArg, int>& AddIntArgument(const std::string& fullname, const std::string& description = "");
    ArgBuilder<IntArg, int>& AddIntArgument(char nickname, const std::string& fullname, const std::string& description = "");
    ArgBuilder<StringArg, std::string>& AddStringArgument(const std::string& fullname, const std::string& description = "");
    ArgBuilder<StringArg, std::string>& AddStringArgument(char nickname, const std::string& fullname, const std::string& description = "");
    ArgBuilder<BoolArg, bool>& AddFlag(const std::string& fullname, const std::string& description = "");
    ArgBuilder<BoolArg, bool>& AddFlag(char nickname, const std::string& fullname, const std::string& description = "");
    void AddHelp(char nickname, const std::string& fullname, const std::string& description = "");
    std::string HelpDescription() const;
    bool Help() const;

private:

    void Build();
    bool IsValid() const;
    ArgData* GetArgData(std::string_view name);
    template<typename T>
    Argument<T>* GetArgument(std::string_view name) {
        auto iterator = args_data.find(name);
        if (iterator == args_data.end()) {
            return nullptr;
        }
        Argument<T>* p_arg = dynamic_cast<Argument<T>*>(args_data.find(name)->second);
        return p_arg;
    }

    const char kShortArgPrefix = '-';
    const std::string kLongArgPrefix = "--";

    std::string name = "";
    bool asked_for_help = false;
    BoolArg* help = nullptr;

    std::map<std::string, ArgData*, std::less<>> args_data;
    std::vector<ArgData*> positional;
    std::vector<IArgumentBuilder*> builders;
};

} // namespace ArgumentParser
