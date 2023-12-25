#pragma once

#include <string>
#include <vector>

namespace ArgumentData {
 
enum class ParseStatus {
    kParsedSuccessfully,
    kNotParsed,
    kInvalidArguments,
    kPartlyParsed
};

class ArgData {
public:
    virtual ~ArgData() = default;

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

    bool is_master = true;

    virtual size_t GetStorageSize() const = 0;
    virtual ParseStatus Parse(const std::vector<std::string>& argv, int& iterator) = 0;
    virtual bool IsValid() const = 0;
};

template<typename T>
class Argument : public ArgData {
public:

    virtual ParseStatus Parse(const std::vector<std::string>& argv, int& iterator) override = 0;

    virtual ~Argument() override {
        DeleteStorage();
    }
    Argument() {
        storage.single = nullptr;
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
    size_t GetStorageSize() const override final {
        return (is_multivalue) ? storage.multi->size() : 1;
    }
    virtual bool IsValid() const override {
        return CheckNoDefault() && CheckMinCount();
    }
    bool CheckNoDefault() const {
        return has_default || was_parsed;
    }
    bool CheckMinCount() const {
        return !is_multivalue || storage.multi->size() >= min_count;
    }
    

    T default_value{};
    union Storage { T* single; std::vector<T>* multi; } storage;
};

} // namespace ArgumentData 
