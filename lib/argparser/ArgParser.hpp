#pragma once

#include "ArgumentData.hpp"
#include "BoolArgument.hpp"
#include "IntArgument.hpp"
#include "StringArgument.hpp"

#include <concepts>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace ArgumentParser {

using namespace ArgumentData;

template <typename ArgT>
concept IsArgument = requires
{
    typename ArgT::ValueType;
    std::is_base_of_v<Argument<typename ArgT::ValueType>, ArgT>;
};

class ArgParser {
public:
    ArgParser(std::string_view id);
    ArgParser(const ArgParser& other) = delete;
    ArgParser& operator=(const ArgParser& other) = delete;
    ~ArgParser();

    bool Parse(int argc, char** argv);
    bool Parse(const std::vector<std::string>& argv);
    bool Parse(const std::vector<std::string_view>& argv);

    template<typename ArgT> requires IsArgument<ArgT>
    Argument<typename ArgT::ValueType>& AddArgument(const std::string& fullname, bool take_param, const std::string& description = "") {
        ArgT* arg = new ArgT; 
        arg->Initialize(fullname, description, take_param);
        PushArgument(arg);
        return *arg;
    }

    template<typename ArgT> requires IsArgument<ArgT>
    Argument<typename ArgT::ValueType>& AddArgument(char nickname, const std::string& fullname, bool take_param, const std::string& description = "") {
        return AddArgument<ArgT>(fullname, description, take_param).AddNickname(nickname);
    }

    void PushArgument(ArgData* arg_ptr);

    template<typename T>
    std::optional<T> GetValue(std::string_view name) {
        Argument<T>* p_arg = GetArgument<T>(name);
        if (!p_arg || p_arg->multivalue_min_count.has_value()) {
            return std::nullopt;
        }
        return p_arg->storage.GetValue();
    }

    template<typename T>
    std::optional<std::vector<T>> GetValues(std::string_view name) {
        Argument<T>* p_arg = GetArgument<T>(name);
        if (!p_arg || !p_arg->multivalue_min_count.has_value()) {
            return std::nullopt;
        }
        return p_arg->storage.GetValues();
    }

    // Built-in types
    Argument<int>& AddIntArgument(const std::string& fullname, const std::string& description = "");
    Argument<int>& AddIntArgument(char nickname, const std::string& fullname, const std::string& description = "");
    Argument<std::string>& AddStringArgument(const std::string& fullname, const std::string& description = "");
    Argument<std::string>& AddStringArgument(char nickname, const std::string& fullname, const std::string& description = "");
    Argument<bool>& AddFlag(const std::string& fullname, const std::string& description = "");
    Argument<bool>& AddFlag(char nickname, const std::string& fullname, const std::string& description = "");
    void AddHelp(char nickname, const std::string& fullname, const std::string& description = "");
    std::string HelpDescription() const;
    bool Help() const;

private:

    void FindPositional();
    bool IsValid() const;
    bool ParseAsPositional(std::string_view arg);
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
    const std::string kSplitter = "--";

    std::string name = "";
    bool asked_for_help = false;
    BoolArg* help = nullptr;

    std::map<std::string, ArgData*, std::less<>> args_data;
    std::vector<ArgData*> positional;
};

} // namespace ArgumentParser
