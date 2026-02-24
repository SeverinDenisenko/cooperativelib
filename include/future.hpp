#pragma once

#include "error.hpp"
#include "result.hpp"

#include <functional>
#include <variant>

namespace co {

template <typename T>
class future;

template <typename T>
class promice;

template <typename T>
std::pair<future<T>, promice<T>> create_futue_promice_pair() noexcept;

template <typename T>
struct future_promice_control_block {
    size_t refcount { 0 };
    bool ready { false };
    con::result<T> value {};
    std::function<std::monostate(con::result<T>)> continuation {};
};

template <typename T>
class promice {
public:
    promice(const promice& other)
        : control_block_(other.control_block_)
    {
        ++control_block_->refcount;
    }

    ~promice()
    {
        if (control_block_ == nullptr) {
            return;
        }

        --control_block_->refcount;
        if (control_block_->refcount == 0) {
            delete control_block_;
        }
    }

    promice() = delete;

    promice(promice&& other)
    {
        control_block_       = other.control_block_;
        other.control_block_ = nullptr;
    }

    void resolve(con::result<T>&& value)
    {
        control_block_->ready = true;
        control_block_->value = std::move(value);
        if (control_block_->continuation) {
            control_block_->continuation(control_block_->value);
            control_block_->continuation = {};
        }
    }

    void set_value(T value)
    {
        resolve(con::result<T>(std::move(value)));
    }

    void set_exception(std::exception_ptr exception)
    {
        resolve(con::result<T>(std::move(exception)));
    }

    template <typename U>
    friend std::pair<future<U>, promice<U>> create_futue_promice_pair() noexcept;

private:
    promice(future_promice_control_block<T>* control_block) noexcept
        : control_block_(control_block)
    {
        ++control_block_->refcount;
    }

    future_promice_control_block<T>* control_block_;
};

template <typename T>
class future {
public:
    ~future()
    {
        if (control_block_ == nullptr) {
            return;
        }

        --control_block_->refcount;
        if (control_block_->refcount == 0) {
            delete control_block_;
        }
    }

    future(const future&) = delete;
    future()              = delete;

    future(future&& other)
    {
        control_block_       = other.control_block_;
        other.control_block_ = nullptr;
    }

    template <typename U>
    friend std::pair<future<U>, promice<U>> create_futue_promice_pair() noexcept;

    bool ready()
    {
        return control_block_->ready;
    }

    bool has_exception()
    {
        return control_block_->value.has_exception();
    }

    bool has_value()
    {
        return control_block_->value.has_value() && control_block_->ready;
    }

    T& get()
    {
        if (!control_block_->ready) {
            throw con::error("future is not ready");
        }

        return control_block_->value.value();
    }

    template <typename Ret>
    future<Ret> then(std::function<Ret(con::result<T>)> task);

private:
    future(future_promice_control_block<T>* control_block) noexcept
        : control_block_(control_block)
    {
        ++control_block_->refcount;
    }

    future_promice_control_block<T>* control_block_;
};

template <typename T>
std::pair<future<T>, promice<T>> create_futue_promice_pair() noexcept
{
    future_promice_control_block<T>* contol_block = new future_promice_control_block<T>();
    future<T> fut(contol_block);
    promice<T> prom(contol_block);
    return std::pair<future<T>, promice<T>>(std::move(fut), std::move(prom));
}

template <typename Arg>
template <typename Ret>
future<Ret> future<Arg>::then(std::function<Ret(con::result<Arg>)> task)
{
    auto [fut, prom] = create_futue_promice_pair<Ret>();

    control_block_->continuation
        = [prom = std::move(prom), task = std::move(task)](con::result<Arg> arg) mutable -> std::monostate {
        try {
            prom.set_value(task(std::move(arg)));
        } catch (...) {
            prom.set_exception(std::current_exception());
            return std::monostate {};
        }
        return std::monostate {};
    };

    return std::move(fut);
}

} // namespace co
