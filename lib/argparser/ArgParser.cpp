#include "ArgParser.hpp"

namespace ArgumentParser {

void ArgParser::Build() {
    for (auto ptr : builders) {
        const std::string& name = ptr->GetArgumentName();
        ArgData* arg_ptr = ptr->Build();
        args_data[name] = arg_ptr;
        if (arg_ptr->is_positional) {
            positional.push_back(arg_ptr);
        }
    }
}

ArgParser::~ArgParser() {
    for (auto ptr : builders) {
        delete ptr;
    }
    for (auto ptr : args_data) {
        delete ptr.second;
    }
}

bool ArgParser::Parse(int argc, char** argv) {
	return Parse(std::vector<std::string_view>(argv, argv + argc));
}

bool ArgParser::Parse(const std::vector<std::string>& argv) {
    return Parse(std::vector<std::string_view>(argv.begin(), argv.end()));
}

bool ArgParser::Parse(const std::vector<std::string_view>& argv) {
    Build();

    for (int iterator = 1; iterator < argv.size(); ++iterator) {
        bool parsed = false;

        if (argv[iterator].starts_with(kLongArgPrefix)) {
            std::string_view arg_name = argv[iterator].substr(kLongArgPrefix.length());
            std::string_view arg_value;
            bool is_valid = true;
            size_t equal_sign_pos = arg_name.find('=');
            if (equal_sign_pos == std::string_view::npos) {
                is_valid = iterator + 1 < argv.size();
                arg_value = is_valid ? argv[++iterator] : std::string_view{};
            } else {
                arg_value = arg_name.substr(equal_sign_pos + 1);
                arg_name = arg_name.substr(0, equal_sign_pos);
                is_valid = arg_name.size();
            }

            if (ArgData* argdata_ptr = GetArgData(arg_name)) {
                if (argdata_ptr->has_param && is_valid && argdata_ptr->ParseAndSave(arg_value) == ParseStatus::kParsedSuccessfully) {
                    parsed = true;
                    continue;
                }
                if (!argdata_ptr->has_param && argdata_ptr->ParseAndSave("") == ParseStatus::kParsedSuccessfully) {
                    parsed = true;
                    continue;
                }
            }

            return false;
        } else if(argv[iterator].starts_with(kShortArgPrefix)) {
            for (int i = 1; i < argv[iterator].size(); ++i) {
                char arg = argv[iterator][i];
                parsed = false;

                ArgData* argdata = nullptr;
                for (auto& [name, args_data] : args_data) {
                    if (args_data->has_nickname && args_data->nickname == arg) {
                        argdata = args_data;
                        break;
                    }
                }

                if (!argdata) {
                    break;
                } else if (argdata->has_param) {
                    if (i + 1 < argv[iterator].size() && argv[iterator][i + 1] != '=') {
                        return false;
                    }
                    if (i + 1 < argv[iterator].size() && argv[iterator][i + 1] == '=') {
                        std::string_view arg_value = argv[iterator].substr(argv[iterator].find('=') + 1);
                        if (argdata->ParseAndSave(arg_value) != ParseStatus::kParsedSuccessfully) {
                            return false;
                        }
                    } else if (iterator + 1 >= argv.size() || argdata->ParseAndSave(argv[++iterator]) != ParseStatus::kParsedSuccessfully) {
                        return false;
                    }
                    parsed = true;
                    break;
                } else if (argdata->ParseAndSave("") == ParseStatus::kParsedSuccessfully) {
                    parsed = true;
                    continue;
                }

                return false;
            }
        } 
        if (parsed) {
            continue;
        }
        for (ArgData* argdata_ptr : positional) {
            if (argdata_ptr->ParseAndSave(argv[iterator]) == ParseStatus::kParsedSuccessfully) {
                parsed = true;
                break;
            }
        }
        if (!parsed) {
            return false;
        }
    }
    
    return asked_for_help || IsValid();
}

ArgData* ArgParser::GetArgData(std::string_view name) {
    auto iterator = args_data.find(name);
    if (iterator == args_data.end()) {
        return nullptr;
    }
    return iterator->second;
}

bool ArgParser::IsValid() const {
    for (auto& [name, arg] : args_data) {
        if (!arg->Validate()) {
            return false;
        }
    }
    return true;
}

ArgParser::ArgParser(std::string_view name) {
    this->name = name;
}

void ArgParser::PushArgument(ArgData* arg_ptr) {
    args_data[arg_ptr->fullname] = arg_ptr;
    if (arg_ptr->is_positional) {
        positional.push_back(arg_ptr);
    }
}

// Built-in types
ArgBuilder<IntArg, int>& ArgParser::AddIntArgument(const std::string& fullname, const std::string& description) { 
    return AddArgument<IntArg, int>(fullname, true, description); 
}

ArgBuilder<IntArg, int>& ArgParser::AddIntArgument(char nickname, const std::string& fullname, const std::string& description) { 
    return AddIntArgument(fullname, description).AddNickname(nickname); 
}

ArgBuilder<StringArg, std::string>& ArgParser::AddStringArgument(const std::string& fullname, const std::string& description) { 
    return AddArgument<StringArg, std::string>(fullname, true, description); 
}

ArgBuilder<StringArg, std::string>& ArgParser::AddStringArgument(char nickname, const std::string& fullname, const std::string& description) { 
    return AddStringArgument(fullname, description).AddNickname(nickname); 
}

ArgBuilder<BoolArg, bool>& ArgParser::AddFlag(const std::string& fullname, const std::string& description) { 
    return AddArgument<BoolArg, bool>(fullname, false, description).Default(false); 
}

ArgBuilder<BoolArg, bool>& ArgParser::AddFlag(char nickname, const std::string& fullname, const std::string& description) { 
    return AddFlag(fullname, description).AddNickname(nickname); 
}

void ArgParser::AddHelp(char nickname, const std::string& fullname, const std::string& description) { 
    AddFlag(nickname, fullname, description).StoreValue(asked_for_help);
}

bool ArgParser::Help() const {
    return asked_for_help;
}

std::string ArgParser::HelpDescription() const {

    std::stringstream help_description;
    help_description << name << std::endl;
    if (help) {
        help_description << help->description << std::endl;
    }
    help_description << std::endl;

    for (const auto& pair_argname_argdata : args_data) {
        
        auto& argdata = *pair_argname_argdata.second;
        if (argdata.has_nickname) {
            help_description << kShortArgPrefix << argdata.nickname << ',';
        } else { 
            help_description << ' ' << ' ' << ' ';
        }
        help_description << ' ' << ' ';
        help_description << kLongArgPrefix << argdata.fullname << "=<value>,  ";
        help_description << argdata.description << ' ';
        help_description << argdata.Info();
        help_description << std::endl;
    }

    help_description << std::endl;
    if (help) {
        help_description << kShortArgPrefix << help->nickname << ',';
        help_description << ' ' << ' ';
        help_description << kLongArgPrefix << help->fullname << ',' << ' ' << ' ';
        help_description << "Display this help and exit";
        help_description << std::endl;
    }

    return help_description.str();
}

} // namespace ArgumentParser
