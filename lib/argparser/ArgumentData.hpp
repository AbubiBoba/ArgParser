#pragma once

#include <charconv>
#include <optional>
#include <string>
#include <sstream>
#include <typeinfo>
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

    virtual ParseStatus ParseAndSave(std::string_view arg) = 0;
    virtual bool Validate() const = 0;
    virtual std::string Info() const = 0;
    virtual std::string_view GetTypename() const = 0;
};

template<typename T>
class Storage {
public:
    std::optional<T> default_value = std::nullopt;

    ~Storage() {
        DeleteStorage();
    }

    Storage() {
        storage.single = nullptr;
    }

    void Init() {
        storage.single = new T{};
    }

    void Save(const T& value) {
        if (is_multivalue) {
            storage.multi->push_back(value);
        }
        else {
            *storage.single = value;
        }
    }

    void Multivalue() {
        DeleteStorage();
        is_multivalue = true;
        storage.multi = new std::vector<T>;
    }

    void StoreValue(T& external_storage) {
        external_storage = std::move(*storage.single);
        DeleteStorage();
        is_owned = false;
        storage.single = &external_storage;
    }

    void StoreValues(std::vector<T>& external_storage) {
        external_storage = std::move(*storage.multi);
        DeleteStorage();
        is_owned = false;
        storage.multi = &external_storage;
    }

    T& GetValue() {
        return *storage.single;
    }

    const T& GetValue() const {
        return *storage.single;
    }

    const std::vector<T>& GetValues() const {
        return *storage.multi;
    }
private:
    bool is_multivalue = false;
    bool is_owned = true;
    union Pointer { T* single; std::vector<T>* multi; } storage;

    void DeleteStorage() {
        if (is_owned) {
            if (is_multivalue) {
                delete storage.multi;
            }
            else {
                delete storage.single;
            }
        }
    }
};

template<typename T>
class Argument : public ArgData {
public:
    using ValueType = T;
    virtual ParseStatus ParseAndSave(std::string_view arg) override = 0;

    virtual ~Argument() override { }

    virtual std::string_view GetTypename() const override {
        return typeid(T).name();
    }

    void Initialize(const std::string& fullname, const std::string& description, bool takes_param, char nickname = ' ') {
        this->fullname = fullname;
        this->description = description;
        if (nickname != ' ') {
            AddNickname(nickname);
        }
        this->takes_param = takes_param;
        storage.Init();
    }

    Argument<T>& MultiValue(size_t min_cnt = 0) {
        storage.Multivalue();
        multivalue_min_count = min_cnt;
        return *this;
    }

    Argument<T>& Positional() {
        is_positional = true;
        return *this;
    }

    Argument<T>& Default(const T& standard) {
        if (!multivalue_min_count.has_value()) {
            storage.Save(standard);
            storage.default_value = standard;
        }
        return *this;
    }

    Argument<T>& StoreValue(T& external_storage) {
        storage.StoreValue(external_storage);
        return *this;
    }

    Argument<T>& StoreValues(std::vector<T>& external_storage) {
        storage.StoreValues(external_storage);
        return *this;
    }

    Argument<T>& AddNickname(char nickname) {
        this->nickname = nickname;
        return *this;
    }


    Storage<T> storage;

    virtual bool Validate() const override {
        return CheckNoDefault() && CheckMinCount();
    }

    virtual std::string Info() const override {
        std::stringstream info;

        if (storage.default_value.has_value()) {
            info << "[default] ";
        }
        if (multivalue_min_count.has_value()) {
            info << "[repeated, min args = " << multivalue_min_count.value() << "] ";
        }
        return info.str();
    }

protected:

    bool CheckNoDefault() const {
        return storage.default_value.has_value() || was_parsed;
    }

    bool CheckMinCount() const {
        return !multivalue_min_count.has_value() || storage.GetValues().size() >= multivalue_min_count.value();
    }
};

} // namespace ArgumentData 
