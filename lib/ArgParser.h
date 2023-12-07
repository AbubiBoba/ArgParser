#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace Builder {

template<typename T>
struct ArgData {
public:
    ~ArgData() {
        DeleteStorage();
    }
    void DeleteStorage() {
        if (is_master) {
            if (is_multivalue) {
                delete storage.multi;
            } else {
                delete storage.single;
            }
        }
    }
    bool has_nickname = false;
    std::string fullname = "";
    char nickname = ' ';
    std::string description = "";

    bool has_param = false;
    bool was_parsed = false;

    bool is_positional = false;

    bool is_multivalue = false;
    int min_count = 0;

    bool has_default = false;
    T default_value{};

    bool is_master = true;
    union Storage { T* single; std::vector<T>* multi; } storage = nullptr;
};

template<typename T> class ArgBuilder {
public:
    ~ArgBuilder<T>() {
        delete product;
    }
    ArgBuilder<T>& operator=(const ArgBuilder<T>& other) = delete;
    ArgBuilder<T>(const ArgBuilder<T>& other) = delete;
    ArgBuilder<T>(const std::string& fullname, const std::string& description, bool has_param, char nickname = ' ') {
        product = new ArgData<T>;
        product->fullname = fullname;
        product->description = description;
        product->has_nickname = nickname != ' ';
        product->nickname = nickname;
        product->has_param = has_param;
        product->storage.single = new T{};
    }
    ArgBuilder<T>& MultiValue(int min_cnt = 0) {
        product->DeleteStorage();
        product->is_multivalue = true;
        product->storage.multi = new std::vector<T>;
        product->min_count = min_cnt;
        return *this;
    }
    ArgBuilder<T>& Positional() {
        product->is_positional = true;
        return *this;
    }
    ArgBuilder<T>& Default(const T& standard) {
        if (!product->is_multivalue) {
            *product->storage.single = standard;
            product->has_default = true;
            product->default_value = standard;
        }
        return *this;
    }
    ArgBuilder<T>& StoreValue(T& external_storage) {
        external_storage = std::move(*product->storage.single);
        product->DeleteStorage();
        product->is_master = false;
        product->storage.single = &external_storage;
        return *this;
    }
    ArgBuilder<T>& StoreValues(std::vector<T>& external_storage) {
        external_storage = std::move(*product->storage.multi);
        product->DeleteStorage();
        product->is_master = false;
        product->storage.multi = &external_storage;
        return *this;
    }
    [[nodiscard]]
    ArgData<T>* Build() {
        ArgData<T>* to_return = product;
        Reset();
        return to_return;
    }
    const std::string& GetArgumentName() const {
        return product->fullname;
    }
private:
    ArgData<T>* product = nullptr;
    void Reset() {
        product = new ArgData<T>;
    }
};

} // namespace Builder

namespace ArgumentParser {

using namespace Builder;

enum class ParseStatus {
    kParsedSuccessfully,
    kNotParsed,
    kInvalidArguments
};

class ArgParser {
public:
    ArgParser(const std::string& id);
    ArgParser(const ArgParser& other) = delete;
    ArgParser& operator=(const ArgParser& other) = delete;
    ~ArgParser();
    const std::string& Log();
    bool Parse(int argc, char** argv);
    bool Parse(std::vector<std::string> argv);
    void BuildAll();
    template<typename T>
    bool Validate(const std::map<std::string, ArgData<T>*>& map_argdata) {
        for (const auto& pair_argname_argdata : map_argdata) {
            const ArgData<T>& data = *(pair_argname_argdata.second);
            if (!data.has_default && !data.was_parsed) {
                log = "Argument " + data.fullname + " wasn`t parsed";
                return false;
            }
            if (data.is_multivalue && data.storage.multi->size() < data.min_count) {
                log = "Argument " + data.fullname + " has less parametrs then expected";
                return false;
            }
        }
        return true;
    }

    // Help
    void AddHelp(char nickname, std::string fullname, std::string description = "");
    bool Help() const;
    std::string HelpDescription() const;
    ParseStatus ParseHelp(const std::vector<std::string>& argv, int& iterator);

    // Int
    ArgBuilder<int>& AddIntArgument(std::string fullname, std::string description = "");
    ArgBuilder<int>& AddIntArgument(char nickname, std::string fullname, std::string description = "");
    int GetIntValue(std::string name, int index = 0);
    void BuildInt();
    ParseStatus ParseInt(const std::vector<std::string>& argv, int& iterator);
    bool IsInt(const std::string_view arg) const;

    // String
    ArgBuilder<std::string>& AddStringArgument(std::string fullname, std::string description = "");
    ArgBuilder<std::string>& AddStringArgument(char nickname, std::string fullname, std::string description = "");
    std::string GetStringValue(std::string name, int index = 0);
    void BuildString();
    ParseStatus ParseString(const std::vector<std::string>& argv, int& iterator);

    // Flag
    ArgBuilder<bool>& AddFlag(std::string fullname, std::string description = "");
    ArgBuilder<bool>& AddFlag(char nickname, std::string fullname, std::string description = "");
    bool GetFlag(std::string name, int index = 0);
    void BuildFlag();
    ParseStatus ParseFlag(const std::vector<std::string>& argv, int& iterator);

private:
    std::string id = "";

    const char kShortArgPrefix = '-';
    const std::string kLongArgPrefix = "--";

    bool is_valid = true;
    std::string log = "Log is empty";

    ArgData<std::string>* help = nullptr;

    std::vector<ArgBuilder<int>*> int_builders;
    std::map<std::string, ArgData<int>*> int_data;
    ArgData<int>* int_positional = nullptr;

    std::vector<ArgBuilder<std::string>*> string_builders;
    std::map<std::string, ArgData<std::string>*> string_data;
    ArgData<std::string>* string_positional = nullptr;

    std::vector<ArgBuilder<bool>*> flag_builders;
    std::map<std::string, ArgData<bool>*> flag_data;

    std::vector<void(ArgParser::*)()> builds = { &ArgParser::BuildInt, &ArgParser::BuildString, &ArgParser::BuildFlag };
    std::vector<ParseStatus(ArgParser::*)(const std::vector<std::string>&, int&)> parses = { &ArgParser::ParseHelp, &ArgParser::ParseInt, &ArgParser::ParseString, &ArgParser::ParseFlag };
};

} // namespace ArgumentParser
