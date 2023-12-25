#pragma once

#include "ArgumentData.hpp"
#include "Builder.hpp"
#include "BoolArgument.hpp"
#include "IntArgument.hpp"
#include "StringArgument.hpp"

#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace ArgumentParser {

using namespace Builder;
using namespace ArgumentData;



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
    ArgBuilder<IntArgument::IntArg, int>& AddIntArgument(const std::string& fullname, const std::string& description = "") { return AddArgument<IntArgument::IntArg, int>(fullname, description, true); }
    ArgBuilder<IntArgument::IntArg, int>& AddIntArgument(char nickname, const std::string& fullname, const std::string& description = "") { return AddIntArgument(fullname, description).AddNickname(nickname); }
    ArgBuilder<StringArgument::StringArg, std::string>& AddStringArgument(const std::string& fullname, const std::string& description = "") { return AddArgument<StringArgument::StringArg, std::string>(fullname, description, true); }
    ArgBuilder<StringArgument::StringArg, std::string>& AddStringArgument(char nickname, const std::string& fullname, const std::string& description = "") { return AddStringArgument(fullname, description).AddNickname(nickname); }
    ArgBuilder<BoolArgument::BoolArg, bool>& AddFlag(const std::string& fullname, const std::string& description = "") { return AddArgument<BoolArgument::BoolArg, bool>(fullname, description).Default(false); }
    ArgBuilder<BoolArgument::BoolArg, bool>& AddFlag(char nickname, const std::string& fullname, const std::string& description = "") { return AddFlag(fullname, description).AddNickname(nickname); }

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
