#pragma once

#include "ArgumentData.hpp"

#include <string>
#include <type_traits>
#include <vector>

namespace Builder {

using namespace ArgumentData;

class IArgumentBuilder {
public:
    virtual ~IArgumentBuilder() { }
    [[nodiscard]] virtual ArgData* Build() = 0;
    virtual const std::string & GetArgumentName() const = 0;
};

template<class ArgT, typename T>
    requires(std::is_base_of<Argument<T>, ArgT>::value)
class ArgBuilder : public IArgumentBuilder {
public:
    ~ArgBuilder() override {
        delete product;
    }

    ArgBuilder<ArgT, T>& operator=(const ArgBuilder<ArgT, T>& other) = delete;
    ArgBuilder(const ArgBuilder<ArgT, T>& other) = delete;

    ArgBuilder(const std::string& fullname, const std::string& description, bool has_param, char nickname = ' ') {
        Reset();
        product->fullname = fullname;
        product->description = description;
        product->has_nickname = (nickname != ' ');
        product->nickname = nickname;
        product->has_param = has_param;
        product->storage.single = new T{};
    }

    ArgBuilder<ArgT, T>& MultiValue(size_t min_cnt = 0) {
        product->DeleteStorage();
        product->is_multivalue = true;
        product->storage.multi = new std::vector<T>;
        product->min_count = min_cnt;
        return *this;
    }

    ArgBuilder<ArgT, T>& Positional() {
        product->is_positional = true;
        return *this;
    }

    ArgBuilder<ArgT, T>& Default(const T& standard) {
        if (!product->is_multivalue) {
            *product->storage.single = standard;
            product->has_default = true;
            product->default_value = standard;
        }
        return *this;
    }

    ArgBuilder<ArgT, T>& StoreValue(T& external_storage) {
        external_storage = std::move(*product->storage.single);
        product->DeleteStorage();
        product->is_owned = false;
        product->storage.single = &external_storage;
        return *this;
    }

    ArgBuilder<ArgT, T>& StoreValues(std::vector<T>& external_storage) {
        external_storage = std::move(*product->storage.multi);
        product->DeleteStorage();
        product->is_owned = false;
        product->storage.multi = &external_storage;
        return *this;
    }

    ArgBuilder<ArgT, T>& AddNickname(char nickname) {
        product->has_nickname = true;
        product->nickname = nickname;
        return *this;
    }

    [[nodiscard]]
    Argument<T>* Build() override {
        Argument<T>* to_return = product;
        Reset();
        return to_return;
    }

    virtual const std::string& GetArgumentName() const override {
        return product->fullname;
    }
private:
    Argument<T>* product = nullptr;

    void Reset() {
        product = new ArgT{};
    }
};

} // namespace Builder
