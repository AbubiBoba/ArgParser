#pragma once

#include <charconv>
#include <optional>
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

    std::optional<char> nickname = std::nullopt;
    std::string fullname;
    std::string description;

    bool takes_param = false;
    bool was_parsed = false;

    bool is_positional = false;

    std::optional<int> multivalue_min_count = std::nullopt;

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
            if (multivalue_min_count.has_value()) {
                delete storage.multi;
            } else {
                delete storage.single;
            }
        }
    }

    size_t GetStorageSize() const override final {
        return (multivalue_min_count.has_value()) ? storage.multi->size() : 1;
    }

    virtual bool Validate() const override {
        return CheckNoDefault() && CheckMinCount();
    }

    bool CheckNoDefault() const {
        return default_value.has_value() || was_parsed;
    }

    bool CheckMinCount() const {
        return !multivalue_min_count.has_value() || storage.multi->size() >= multivalue_min_count.value();
    }

    virtual std::string Info() override {
        std::stringstream info;

        if (default_value.has_value()) {
            info << "[default] ";
        }
        if (multivalue_min_count.has_value()) {
            info << "[repeated, min args = " << multivalue_min_count.value() << "] ";
        }
        return info.str();
    }

    virtual void Save(const T& value) {
        if (multivalue_min_count.has_value()) {
            storage.multi->push_back(value);
        } else {
            *storage.single = value;
        }
    }
    
    std::optional<T> default_value = std::nullopt;

    bool is_owned = true;
    union Storage { T* single; std::vector<T>* multi; } storage;
};

} // namespace ArgumentData 
