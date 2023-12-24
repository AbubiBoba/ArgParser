#include "ArgParser.h"

namespace ArgumentParser {

    using namespace Builder;

    // Help
    void ArgParser::AddHelp(char nickname, std::string fullname, std::string description) {
        delete help;
        help = (new ArgBuilder<bool>(fullname, description, false, nickname))->Build();
    }

    bool ArgParser::Help() const {
        return (help) ? help->was_parsed : false;
    }

    std::string ArgParser::HelpDescription() const {
        std::string endline = "\n";
        std::string help_description;
        help_description += id + endline;
        if (help) {
            help_description += help->description + endline;
        }
        help_description += endline;
        // Int
        for (const auto& pair_argname_argdata : int_data) {
            auto& argdata = *pair_argname_argdata.second;
            help_description += (argdata.has_nickname) ? std::string(1, kShortArgPrefix) + std::string(1, argdata.nickname) + "," : std::string(3, ' ');
            help_description += std::string(2, ' ');
            help_description += kLongArgPrefix + argdata.fullname + "=<int>,  ";
            help_description += argdata.description + " ";
            if (argdata.has_default) {
                help_description += "[default = " + std::to_string(argdata.default_value) + "] ";
            }
            if (argdata.is_multivalue) {
                help_description += "[repeated, min args = " + std::to_string(argdata.min_count) + "] ";
            }
            help_description += endline;
        }
        // String
        for (const auto& pair_argname_argdata : string_data) {
            auto& argdata = *pair_argname_argdata.second;
            help_description += (argdata.has_nickname) ? std::string(1, kShortArgPrefix) + std::string(1, argdata.nickname) + "," : std::string(3, ' ');
            help_description += std::string(2, ' ');
            help_description += kLongArgPrefix + argdata.fullname + "=<string>,  ";
            help_description += argdata.description + " ";
            if (argdata.has_default) {
                help_description += "[default = " + argdata.default_value + "] ";
            }
            if (argdata.is_multivalue) {
                help_description += "[repeated, min args = " + std::to_string(argdata.min_count) + "] ";
            }
            help_description += endline;
        }
        // Flag
        for (const auto& pair_argname_argdata : flag_data) {
            auto& argdata = *pair_argname_argdata.second;
            help_description += (argdata.has_nickname) ? std::string(1, kShortArgPrefix) + std::string(1, argdata.nickname) + "," : std::string(3, ' ');
            help_description += std::string(2, ' ');
            help_description += kLongArgPrefix + argdata.fullname + ",  ";
            help_description += argdata.description + " ";
            if (argdata.has_default) {
                help_description += "[default = " + (argdata.default_value ? std::string("true") : std::string("false")) + "] ";
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
            help_description += kLongArgPrefix + help->fullname + std::string(1, ',') + std::string(2, ' ') ;
            help_description += "Display this help and exit";
            help_description += endline;
        }

        return help_description;
    }

    ParseStatus ArgParser::ParseHelp(const std::vector<std::string>& argv, int& iterator) {

        const std::string& arg = argv[iterator];
        if (help && (arg == kShortArgPrefix + std::string(1, help->nickname) || arg == kLongArgPrefix + help->fullname)) {
            help->was_parsed = true;
            return ParseStatus::kParsedSuccessfully;
        }

        return ParseStatus::kNotParsed;
    }

    // Int
    ArgBuilder<int>& ArgParser::AddIntArgument(std::string fullname, std::string description) {
        return *int_builders.emplace_back(new ArgBuilder<int>(fullname, description, true));
    }

    ArgBuilder<int>& ArgParser::AddIntArgument(char nickname, std::string fullname, std::string description) {
        return *int_builders.emplace_back(new ArgBuilder<int>(fullname, description, true, nickname));
    }

    int ArgParser::GetIntValue(std::string name, int index) {
        if (!int_data[name]->is_multivalue) {
            return *(int_data[name]->storage.single);
        }

        return (*(int_data[name]->storage.multi))[index];
    }

    void ArgParser::BuildInt() {
        for (auto ptr : int_builders) {
            const std::string& name = ptr->GetArgumentName();
            int_data[name] = ptr->Build();
            if (int_data[name]->is_positional) {
                int_positional = int_data[name];
            }
        }
    }

    bool ArgParser::IsInt(const std::string_view arg) const {
        bool is_int = std::isdigit(arg[0]) || arg[0] == '-';
        for (int i = 1; i < arg.size(); ++i) {
            is_int = is_int && std::isdigit(arg[i]);
        }
        return is_int;
    }

    ParseStatus ArgParser::ParseInt(const std::vector<std::string>& argv, int& iterator) {

        const std::string& arg = argv[iterator];
        auto save_value = [](ArgData<int>* data, int value) {
            data->was_parsed = true;
            if (data->is_multivalue) {
                data->storage.multi->push_back(value);
            }
            else {
                *(data->storage.single) = value;
            }
        };

        if (int_positional && IsInt({ arg.begin(), arg.end() })) {
            save_value(int_positional, std::stoi(arg));
            return ParseStatus::kParsedSuccessfully;
        }

        for (auto& pair_argname_argdata : int_data) {

            auto& argdata = *pair_argname_argdata.second;
            bool take_next = false;
            if (argdata.has_nickname && (arg == kShortArgPrefix + std::string(1, argdata.nickname))) {
                take_next = true;
            }
            if (arg == kLongArgPrefix + argdata.fullname) {
                take_next = true;
            }

            if (take_next && iterator + 1 < argv.size() && IsInt(argv[iterator + 1])) {
                save_value(&argdata, std::stoi(argv[++iterator]));
                return ParseStatus::kParsedSuccessfully;
            } else if (take_next) {
                log = std::string((iterator + 1 < argv.size()) ? "Invalid int argument was provided" : ("Missed parametr for int argument " + argdata.fullname)) + ": " + argv[iterator + 1];
                return ParseStatus::kInvalidArguments;
            }

            std::string prefix = kLongArgPrefix + argdata.fullname + "=";
            if (arg.starts_with(prefix) && arg.size() > prefix.size() && IsInt({ arg.begin() + prefix.size(), arg.end()})) {
                save_value(&argdata, std::stoi(std::string{ arg.begin() + prefix.size(), arg.end() }));
                return ParseStatus::kParsedSuccessfully;
            }

            if (!argdata.has_nickname) {
                continue;
            }
            
            prefix = kShortArgPrefix + argdata.nickname + "=";
            if (arg.starts_with(prefix) && arg.size() > prefix.size() && IsInt({ arg.begin() + prefix.size(), arg.end() })) {
                save_value(&argdata, std::stoi(std::string{ arg.begin() + prefix.size(), arg.end() }));
                return ParseStatus::kParsedSuccessfully;
            }
        }

        return ParseStatus::kNotParsed;
    }

    // String
    ArgBuilder<std::string>& ArgParser::AddStringArgument(std::string fullname, std::string description) {
        return *string_builders.emplace_back(new ArgBuilder<std::string>(fullname, description, true));
    }

    ArgBuilder<std::string>& ArgParser::AddStringArgument(char nickname, std::string fullname, std::string description) {
        return *string_builders.emplace_back(new ArgBuilder<std::string>(fullname, description, true, nickname));
    }

    std::string ArgParser::GetStringValue(std::string name, int index) {
        if (!string_data[name]->is_multivalue) {
            return *(string_data[name]->storage.single);
        }

        return (*(string_data[name]->storage.multi))[index];
    }

    void ArgParser::BuildString() {
        for (auto ptr : string_builders) {
            const std::string& name = ptr->GetArgumentName();
            string_data[name] = ptr->Build();
            if (string_data[name]->is_positional) {
                string_positional = string_data[name];
            }
        }
    }

    ParseStatus ArgParser::ParseString(const std::vector<std::string>& argv, int& iterator) {

        const std::string& arg = argv[iterator];
        auto save_value = [](ArgData<std::string>* data, std::string value) {
            data->was_parsed = true;
            if (data->is_multivalue) {
                data->storage.multi->push_back(value);
            }
            else {
                *(data->storage.single) = value;
            }
        };

        for (auto& pair_argname_argdata : string_data) {

            auto& argdata = *pair_argname_argdata.second;
            bool take_next = false;
            if (argdata.has_nickname && (arg == kShortArgPrefix + std::string(1, argdata.nickname))) {
                take_next = true;
            }
            if (arg == kLongArgPrefix + argdata.fullname) {
                take_next = true;
            }

            if (take_next && iterator + 1 < argv.size()) {
                save_value(&argdata, argv[++iterator]);
                return ParseStatus::kParsedSuccessfully;
            }
            else if (take_next) {
                log = "Missed parametr for int argument " + argdata.fullname + ": " + argv[iterator + 1];
                return ParseStatus::kInvalidArguments;
            }

            std::string prefix = kLongArgPrefix + argdata.fullname + "=";
            if (arg.starts_with(prefix) && arg.size() > prefix.size()) {
                save_value(&argdata, std::string{ arg.begin() + prefix.size(), arg.end() });
                return ParseStatus::kParsedSuccessfully;
            }

            if (!argdata.has_nickname) {
                continue;
            }

            prefix = std::string(1, kShortArgPrefix) + std::string(1, argdata.nickname) + "=";
            if (arg.starts_with(prefix) && arg.size() > prefix.size()) {
                save_value(&argdata, std::string{ arg.begin() + prefix.size(), arg.end() });
                return ParseStatus::kParsedSuccessfully;
            }
        }

        if (string_positional) {
            save_value(string_positional, arg);
            return ParseStatus::kParsedSuccessfully;
        }

        return ParseStatus::kNotParsed;
    }

    // Flag
    ArgBuilder<bool>& ArgParser::AddFlag(std::string fullname, std::string description) {
        return *flag_builders.emplace_back(new ArgBuilder<bool>(fullname, description, false));
    }

    ArgBuilder<bool>& ArgParser::AddFlag(char nickname, std::string fullname, std::string description) {
        return *flag_builders.emplace_back(new ArgBuilder<bool>(fullname, description, false, nickname));
    }

    bool ArgParser::GetFlag(std::string name, int index) {
        if (!flag_data[name]->is_multivalue) {
            return *(flag_data[name]->storage.single);
        }
        return (*(flag_data[name]->storage.multi))[index];
    }

    void ArgParser::BuildFlag() {
        for (auto ptr : flag_builders) {
            const std::string& name = ptr->GetArgumentName();
            flag_data[name] = ptr->Build();
            if (!flag_data[name]->has_default) {
                flag_data[name]->has_default = true;
                flag_data[name]->default_value = false;
                *flag_data[name]->storage.single = false;
            }
        }
    }

    ParseStatus ArgParser::ParseFlag(const std::vector<std::string>& argv, int& iterator) {
        const std::string& arg = argv[iterator];
        bool partly_parsed = false;

        for (auto& pair_argname_argdata : flag_data) {

            auto& argdata = *pair_argname_argdata.second;

            if (argdata.has_nickname && (arg == kShortArgPrefix + std::string(1, argdata.nickname))) {
                argdata.was_parsed = true;
                *(argdata.storage.single) = !*(argdata.storage.single);
                return ParseStatus::kParsedSuccessfully;
            }

            if (arg == kLongArgPrefix + argdata.fullname) {
                argdata.was_parsed = true;
                *(argdata.storage.single) = !*(argdata.storage.single);
                return ParseStatus::kParsedSuccessfully;
            }

            if (argdata.has_nickname && arg.starts_with(kShortArgPrefix) && arg.find(argdata.nickname) != std::string::npos) {
                argdata.was_parsed = true;
                *(argdata.storage.single) = !*(argdata.storage.single);
                partly_parsed = true;
            }
        }

        return partly_parsed ? ParseStatus::kParsedSuccessfully : ParseStatus::kNotParsed;
    }

    ArgParser::~ArgParser() {
        for (auto ptr : int_builders) {
            delete ptr;
        }
        for (auto ptr : int_data) {
            delete ptr.second;
        }
        for (auto ptr : string_builders) {
            delete ptr;
        }
        for (auto ptr : string_data) {
            delete ptr.second;
        }
        for (auto ptr : flag_builders) {
            delete ptr;
        }
        for (auto ptr : flag_data) {
            delete ptr.second;
        }
    }

    void ArgParser::BuildAll() {
        for (auto build : builds) {
            (this->*build)();
        }
    }

	bool ArgParser::Parse(int argc, char** argv) {
		return Parse(std::vector<std::string>(argv, argv + argc));
	}

	bool ArgParser::Parse(std::vector<std::string> argv) {
		BuildAll();
		
        for (int iterator = 1; iterator < argv.size(); ++iterator) {
            bool was_parsed = false;
            for (auto parse : parses) {
                ParseStatus parse_status = (this->*parse)(argv, iterator);
                if (parse_status == ParseStatus::kInvalidArguments) {
                    return false;
                }
                if (parse_status == ParseStatus::kParsedSuccessfully) {
                    was_parsed = true;
                    break;
                }
            }
            if (!was_parsed) {
                log = "[Error]: Can`t parse argument: " + argv[iterator];
                return false;
            }
        }
		
        return Help() || (Validate(int_data) && Validate(string_data) && Validate(flag_data));
	}

	ArgParser::ArgParser(const std::string& id) {
		this->id = id;
	}

    const std::string& ArgParser::Log() {
        return this->log;
    }

}
