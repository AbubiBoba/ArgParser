#pragma once

#include <charconv>
#include <string>
#include <sstream>
#include <vector>

namespace ArgumentData {
 
enum class ParseStatus {
    kParsedSuccessfully,
    kNotParsed,
    kInvalidArguments
};

class ArgData {
public:
    virtual ~ArgData() = default;

    bool has_nickname = false;
    char nickname = ' ';
    std::string fullname;
    std::string description;

    bool has_param = false;
    bool was_parsed = false;

    bool is_positional = false;

    bool is_multivalue = false;
    int min_count = 0;

    virtual size_t GetStorageSize() const = 0;
    virtual ParseStatus ParseAndSave(std::string_view arg) = 0;
    virtual bool Validate() const = 0;
    virtual std::string Info() = 0;
};

template<typename T>
class Argument : public ArgData {
public:

    virtual ParseStatus ParseAndSave(std::string_view arg) override = 0;

    virtual ~Argument() override {
        DeleteStorage();
    }

    Argument() {
        storage.single = nullptr;
    }

    void DeleteStorage() {
        if (is_owned) {
            if (is_multivalue) {
                delete storage.multi;
            } else {
                delete storage.single;
            }
        }
    }

    size_t GetStorageSize() const override final {
        return (is_multivalue) ? storage.multi->size() : 1;
    }

    virtual bool Validate() const override {
        return CheckNoDefault() && CheckMinCount();
    }

    bool CheckNoDefault() const {
        return has_default || was_parsed;
    }

    bool CheckMinCount() const {
        return !is_multivalue || storage.multi->size() >= min_count;
    }

    virtual std::string Info() override {
        std::stringstream info;

        if (has_default) {
            info << "[default] ";
        }
        if (is_multivalue) {
            info << "[repeated, min args = " << min_count << "] ";
        }
        return info.str();
    }
    
    bool has_default = false;
    T default_value{};

    bool is_owned = true;
    union Storage { T* single; std::vector<T>* multi; } storage;
};

} // namespace ArgumentData 
