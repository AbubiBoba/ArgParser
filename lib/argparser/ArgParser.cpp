#include "ArgParser.hpp"

namespace ArgumentParser {

ArgParser::~ArgParser() {
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
    FindPositional();
    bool met_splitter = false;
    for (int iterator = 1; iterator < argv.size(); ++iterator) {
        bool parsed = false;

        if (argv[iterator] == kSplitter) {
            met_splitter = true;
            continue;
        }

        if (met_splitter) {
            if (!ParseAsPositional(argv[iterator])) {
                return false;
            } else { 
                continue;
            }
        }

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
                if (argdata_ptr->takes_param && is_valid && argdata_ptr->ParseAndSave(arg_value) == ParseStatus::kParsedSuccessfully) {
                    parsed = true;
                    continue;
                }
                if (!argdata_ptr->takes_param && argdata_ptr->ParseAndSave("") == ParseStatus::kParsedSuccessfully) {
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
                    if (args_data->nickname == arg) {
                        argdata = args_data;
                        break;
                    }
                }

                if (!argdata) {
                    break;
                } else if (argdata->takes_param) {
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
        
        if (!parsed && !ParseAsPositional(argv[iterator])) {
            return false;
        }
    }
    
    return asked_for_help || IsValid();
}

bool ArgParser::ParseAsPositional(std::string_view arg) {
    for (ArgData* argdata_ptr : positional) {
        if ((!argdata_ptr->was_parsed || argdata_ptr->multivalue_min_count.has_value()) 
            && argdata_ptr->ParseAndSave(arg) == ParseStatus::kParsedSuccessfully) {
            return true;
        }
    }
    return false;
}

ArgData* ArgParser::GetArgData(std::string_view name) {
    auto iterator = args_data.find(name);
    if (iterator == args_data.end()) {
        return nullptr;
    }
    return iterator->second;
}

void ArgParser::FindPositional() {
    positional.clear();
    for (auto& [name, arg_ptr] : args_data) {
        if (arg_ptr->is_positional) {
            positional.push_back(arg_ptr);
        }
    }
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
}

// Built-in types
Argument<int>& ArgParser::AddIntArgument(const std::string& fullname, const std::string& description) {
    return AddArgument<IntArg>(fullname, true, description); 
}

Argument<int>& ArgParser::AddIntArgument(char nickname, const std::string& fullname, const std::string& description) {
    return AddIntArgument(fullname, description).AddNickname(nickname); 
}

Argument<std::string>& ArgParser::AddStringArgument(const std::string& fullname, const std::string& description) {
    return AddArgument<StringArg>(fullname, true, description); 
}

Argument<std::string>& ArgParser::AddStringArgument(char nickname, const std::string& fullname, const std::string& description) {
    return AddStringArgument(fullname, description).AddNickname(nickname); 
}

Argument<bool>& ArgParser::AddFlag(const std::string& fullname, const std::string& description) {
    return AddArgument<BoolArg>(fullname, false, description).Default(false); 
}

Argument<bool>& ArgParser::AddFlag(char nickname, const std::string& fullname, const std::string& description) {
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

    for (const auto& [name, argdata] : args_data) {
        if (argdata->nickname.has_value()) {
            help_description << kShortArgPrefix << argdata->nickname.value() << ',';
        } else { 
            help_description << ' ' << ' ' << ' ';
        }
        help_description << ' ' << ' ';
        help_description << kLongArgPrefix << argdata->fullname;
        if (argdata->GetTypename() != "") {
            help_description << "=<" << argdata->GetTypename() << ">";
        }
        help_description << ",  ";
        help_description << argdata->description << ' ';
        help_description << argdata->Info();
        help_description << std::endl;
    }

    help_description << std::endl;
    if (help) {
        help_description << kShortArgPrefix << help->nickname.value() << ',';
        help_description << ' ' << ' ';
        help_description << kLongArgPrefix << help->fullname << ',' << ' ' << ' ';
        help_description << "Display this help and exit";
        help_description << std::endl;
    }

    return help_description.str();
}

} // namespace ArgumentParser
