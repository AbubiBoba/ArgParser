#include "ArgParser.hpp"

namespace ArgumentParser {

void ArgParser::Build() {
    for (auto ptr : builders) {
        const std::string& name = ptr->GetArgumentName();
        ArgData* arg_ptr = ptr->Build();
        arg_data[name] = arg_ptr;
        if (arg_ptr->is_positional) {
            positional.push_back(arg_ptr);
        }
    }
}

ArgParser::~ArgParser() {
    for (auto ptr : builders) {
        delete ptr;
    }
    for (auto ptr : arg_data) {
        delete ptr.second;
    }
}

bool ArgParser::Parse(int argc, char** argv) {
	return Parse(std::vector<std::string>(argv, argv + argc));
}

bool ArgParser::Parse(std::vector<std::string> argv) {

    Build();
    std::vector<bool> not_parsed(argv.size());

    for (int iterator = 1; iterator < argv.size(); ++iterator) {
        bool was_parsed = false;
        for (auto& [name, arg] : arg_data) {
            if (arg->is_positional) {
                continue;
            }
            ParseStatus parse_status = arg->Parse(argv, iterator);
                
            if (parse_status == ParseStatus::kInvalidArguments) {
                return false;
            }
            if (parse_status == ParseStatus::kParsedSuccessfully) {
                was_parsed = true;
                break;
            }
            if (parse_status == ParseStatus::kPartlyParsed) {
                was_parsed = true;
            }
        }
        if (!was_parsed) {
            not_parsed[iterator] = true;
        }
    }

    for (int iterator = 1; iterator < argv.size(); ++iterator) {
        if (!not_parsed[iterator]) {
            continue;
        }
        bool was_parsed = false;
        for (auto& [name, arg] : arg_data) {
            if (!arg->is_positional) {
                continue;
            }
            ParseStatus parse_status = arg->Parse(argv, iterator);

            if (parse_status == ParseStatus::kInvalidArguments) {
                return false;
            }
            if (parse_status == ParseStatus::kParsedSuccessfully) {
                was_parsed = true;
                break;
            }
        }
        if (was_parsed) {
            not_parsed[iterator] = false;
        }
    }
    for (int iterator = 1; iterator < argv.size(); ++iterator) {
        if (not_parsed[iterator]) {
            return false;
        }
    }
		
    return IsValid() || asked_for_help;
}

bool ArgParser::IsValid() const {
    for (auto& [name, arg] : arg_data) {
        if (!arg->IsValid()) {
            return false;
        }
    }
    return true;
}

ArgParser::ArgParser(const std::string& name) {
    this->name = name;

}

// Built-in types
ArgBuilder<IntArgument::IntArg, int>& ArgParser::AddIntArgument(const std::string& fullname, const std::string& description) { 
    return AddArgument<IntArgument::IntArg, int>(fullname, true, description); 
}

ArgBuilder<IntArgument::IntArg, int>& ArgParser::AddIntArgument(char nickname, const std::string& fullname, const std::string& description) { 
    return AddIntArgument(fullname, description).AddNickname(nickname); 
}

ArgBuilder<StringArgument::StringArg, std::string>& ArgParser::AddStringArgument(const std::string& fullname, const std::string& description) { 
    return AddArgument<StringArgument::StringArg, std::string>(fullname, true, description); 
}

ArgBuilder<StringArgument::StringArg, std::string>& ArgParser::AddStringArgument(char nickname, const std::string& fullname, const std::string& description) { 
    return AddStringArgument(fullname, description).AddNickname(nickname); 
}

ArgBuilder<BoolArgument::BoolArg, bool>& ArgParser::AddFlag(const std::string& fullname, const std::string& description) { 
    return AddArgument<BoolArgument::BoolArg, bool>(fullname, false, description).Default(false); 
}

ArgBuilder<BoolArgument::BoolArg, bool>& ArgParser::AddFlag(char nickname, const std::string& fullname, const std::string& description) { 
    return AddFlag(fullname, description).AddNickname(nickname); 
}

void ArgParser::AddHelp(char nickname, const std::string& fullname, const std::string& description) { 
    AddFlag(nickname, fullname, description).StoreValue(asked_for_help);
}

bool ArgParser::Help() const {
    return asked_for_help;
}

std::string ArgParser::HelpDescription() const {

    std::string help_description;
    help_description += name + endline;
    if (help) {
        help_description += help->description + endline;
    }
    help_description += endline;

    for (const auto& pair_argname_argdata : arg_data) {
        auto& argdata = *pair_argname_argdata.second;
        help_description += (argdata.has_nickname) ? std::string(1, kShortArgPrefix) + std::string(1, argdata.nickname) + "," : std::string(3, ' ');
        help_description += std::string(2, ' ');
        help_description += kLongArgPrefix + argdata.fullname + "=<value>,  ";
        help_description += argdata.description + " ";
        if (argdata.has_default) {
            help_description += "[default] ";
        }
        if (argdata.is_multivalue) {
            help_description += "[repeated, min args = " + std::to_string(argdata.min_count) + "] ";
        }
        help_description += endline;
    }

    help_description += endline;
    if (help) {
        help_description += std::string(1, kShortArgPrefix) + std::string(1, help->nickname) + ",";
        help_description += std::string(2, ' ');
        help_description += kLongArgPrefix + help->fullname + std::string(1, ',') + std::string(2, ' ');
        help_description += "Display this help and exit";
        help_description += endline;
    }

    return help_description;
}

} // namespace ArgumentParser
