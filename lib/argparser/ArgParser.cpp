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
		
        for (int iterator = 1; iterator < argv.size(); ++iterator) {
            bool was_parsed = false;
            for (auto& [name, arg] : arg_data) {
                ParseStatus parse_status = arg->Parse(argv, iterator);
                
                if (parse_status == ParseStatus::kInvalidArguments) {
                    return false;
                }
                if (parse_status == ParseStatus::kParsedSuccessfully) {
                    was_parsed = true;
                    break;
                }
            }
            if (!was_parsed) {
                return false;
            }
        }
		
        return IsValid();
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
}
