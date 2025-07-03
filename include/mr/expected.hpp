#pragma once
#include <type_traits>
#include <stdexcept>
#include <utility>
#include <variant>
#include <initializer_list>

namespace mr {

#ifndef MR_ASSERT
#define MR_ASSERT(cond) if (!(cond)) { throw std::logic_error("assertion failure"); }
#endif

// tag types
struct unexpect_t { explicit unexpect_t() = default; };
struct in_place_t  { explicit in_place_t()  = default; };

inline constexpr unexpect_t unexpect{};
inline constexpr in_place_t  in_place{};

// unexpected – holds only E
template<class E>
class unexpected {
public:
    unexpected() = delete;
    unexpected(const E& e) : err_(e) {}
    unexpected(E&& e)      : err_(std::move(e)) {}

    // defaults for copy/move/destruct:
    unexpected(const unexpected&)     = default;
    unexpected(unexpected&&) noexcept = default;
    unexpected& operator=(const unexpected&)     = default;
    unexpected& operator=(unexpected&&) noexcept = default;
    ~unexpected() = default;

    const E& error() const & { return err_; }
          E& error() &       { return err_; }

private:
    E err_;
};

template<class E>
unexpected<std::decay_t<E>> make_unexpected(E&& e) {
    return unexpected<std::decay_t<E>>(std::forward<E>(e));
}

// primary expected<T,E>
template<class T, class E>
class expected {
    using storage_t = std::variant<T, unexpected<E>>;
    storage_t storage_;

public:
    // constructors
    // Conditionally enabled default constructor
    template <typename U = T,
              typename = std::enable_if_t<std::is_default_constructible_v<U>>>
    expected() : storage_(std::in_place_index_t<0>{}) {}

    expected(const T& v) : storage_(std::in_place_index_t<0>{}, v) {}
    expected(T&& v) : storage_(std::in_place_index_t<0>{}, std::move(v)) {}

    expected(unexpect_t, const E& e)
        : storage_(std::in_place_index_t<1>{}, e) {}
    expected(unexpect_t, E&& e)
        : storage_(std::in_place_index_t<1>{}, std::move(e)) {}

    expected(const unexpected<E>& u)
        : storage_(std::in_place_index_t<1>{}, u) {}
    expected(unexpected<E>&& u)
        : storage_(std::in_place_index_t<1>{}, std::move(u)) {}

    // in_place constructors
    template<class... Args>
    explicit expected(in_place_t, Args&&... args)
        : storage_(std::in_place_index_t<0>{}, std::forward<Args>(args)...) {}

    template<class U, class... Args>
    explicit expected(in_place_t, std::initializer_list<U> il, Args&&... args)
        : storage_(std::in_place_index_t<0>{}, il, std::forward<Args>(args)...) {}

    // special members
    expected(const expected&) = default;
    expected(expected&&) noexcept = default;
    expected& operator=(const expected&) = default;
    expected& operator=(expected&&) noexcept = default;
    ~expected() = default;

    // bool operator
    explicit operator bool() const noexcept { return has_value(); }
    bool has_value() const noexcept { return storage_.index() == 0; }

    // value access
    T& operator*() & {
        MR_ASSERT(has_value());
        return *std::get_if<0>(&storage_);
    }
    const T& operator*() const & {
        MR_ASSERT(has_value());
        return *std::get_if<0>(&storage_);
    }
    T&& operator*() && {
        MR_ASSERT(has_value());
        return std::get<0>(std::move(storage_));
    }
    const T&& operator*() const&& {
        MR_ASSERT(has_value());
        return std::get<0>(std::move(storage_));
    }

    T& value() & {
        MR_ASSERT(has_value());
        return *std::get_if<0>(&storage_);
    }
    const T& value() const & {
        MR_ASSERT(has_value());
        return *std::get_if<0>(&storage_);
    }
    T&& value() && {
        MR_ASSERT(has_value());
        return std::get<0>(std::move(storage_));
    }
    const T&& value() const&& {
        MR_ASSERT(has_value());
        return std::get<0>(std::move(storage_));
    }

    template<class U>
    T value_or(U&& def) const & {
        static_assert(std::is_convertible_v<U, T>,
                      "fallback must convert to T");
        return has_value()
            ? **this
            : static_cast<T>(std::forward<U>(def));
    }
    template<class U>
    T value_or(U&& def) && {
        static_assert(std::is_convertible_v<U, T>,
                      "fallback must convert to T");
        return has_value()
            ? std::move(**this)
            : static_cast<T>(std::forward<U>(def));
    }

    // pointer access
    T* operator->() {
        MR_ASSERT(has_value());
        return &**this;
    }
    const T* operator->() const {
        MR_ASSERT(has_value());
        return &**this;
    }

    // error access
    E& error() & {
        MR_ASSERT(!has_value());
        return std::get<1>(storage_).error();
    }
    const E& error() const & {
        MR_ASSERT(!has_value());
        return std::get<1>(storage_).error();
    }

    // emplace functions
    template<class... Args>
    void emplace(Args&&... args) {
        storage_.template emplace<0>(std::forward<Args>(args)...);
    }

    template<class U, class... Args>
    void emplace(std::initializer_list<U> il, Args&&... args) {
        storage_.template emplace<0>(il, std::forward<Args>(args)...);
    }
};

// void specialization
template<class E>
class expected<void, E> {
    using storage_t = std::variant<std::monostate, unexpected<E>>;
    storage_t storage_{std::in_place_index_t<0>{}};

public:
    expected() = default;

    expected(unexpect_t, const E& e)
        : storage_(std::in_place_index_t<1>{}, e) {}
    expected(unexpect_t, E&& e)
        : storage_(std::in_place_index_t<1>{}, std::move(e)) {}

    expected(const expected&) = default;
    expected(expected&&) noexcept = default;
    expected& operator=(const expected&) = default;
    expected& operator=(expected&&) noexcept = default;
    ~expected() = default;

    explicit operator bool() const noexcept { return has_value(); }
    bool has_value() const noexcept { return storage_.index() == 0; }

    void value() const {
        MR_ASSERT(has_value());
    }

    E& error() & {
        MR_ASSERT(!has_value());
        return std::get<1>(storage_).error();
    }
    const E& error() const & {
        MR_ASSERT(!has_value());
        return std::get<1>(storage_).error();
    }
};

} // namespace mr
