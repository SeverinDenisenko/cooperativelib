#pragma once

#include "error.hpp"
#include "result.hpp"

#include <functional>

namespace co {

struct unit { };

template <typename T>
class future;

template <typename T>
class promise;

template <typename T>
std::pair<future<T>, promise<T>> create_future_promise_pair() noexcept;

template <typename T>
struct future_promise_control_block {
    size_t refcount { 0 };
    bool ready { false };
    con::result<T> value {};
    std::function<unit(con::result<T>)> continuation {};
};

template <typename T>
class promise {
public:
    promise(const promise& other)
        : control_block_(other.control_block_)
    {
        ++control_block_->refcount;
    }

    ~promise()
    {
        if (control_block_ == nullptr) {
            return;
        }

        --control_block_->refcount;
        if (control_block_->refcount == 0) {
            delete control_block_;
        }
    }

    promise()
        : control_block_(new future_promise_control_block<T>)
    {
        ++control_block_->refcount;
    };

    promise(promise&& other)
    {
        control_block_       = other.control_block_;
        other.control_block_ = nullptr;
    }

    future<T> get_future()
    {
        return future<T>(control_block_);
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
    friend std::pair<future<U>, promise<U>> create_future_promise_pair() noexcept;

private:
    promise(future_promise_control_block<T>* control_block) noexcept
        : control_block_(control_block)
    {
        ++control_block_->refcount;
    }

    future_promise_control_block<T>* control_block_;
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

    future(const future&)            = delete;
    future& operator=(const future&) = delete;

    future()
        : control_block_(nullptr) { };

    future(future&& other)
    {
        control_block_       = other.control_block_;
        other.control_block_ = nullptr;
    }

    future<T>& operator=(future&& other)
    {
        if (this == std::addressof(other)) {
            return *this;
        }

        if (control_block_ != nullptr) {
            --control_block_->refcount;
            if (control_block_->refcount == 0) {
                delete control_block_;
            }
        }

        control_block_       = other.control_block_;
        other.control_block_ = nullptr;

        return *this;
    }

    template <typename U>
    friend std::pair<future<U>, promise<U>> create_future_promise_pair() noexcept;

    bool ready() const
    {
        return control_block_->ready;
    }

    bool has_exception() const
    {
        return control_block_->value.has_exception();
    }

    bool has_value() const
    {
        return control_block_->value.has_value() && control_block_->ready;
    }

    T get()
    {
        if (!control_block_->ready) {
            throw con::error("future is not ready");
        }

        return control_block_->value.value();
    }

    template <typename Ret>
    future<Ret> then(std::function<Ret(con::result<T>)> corutine);

    friend class promise<T>;

private:
    future(future_promise_control_block<T>* control_block) noexcept
        : control_block_(control_block)
    {
        ++control_block_->refcount;
    }

    future_promise_control_block<T>* control_block_;
};

template <typename T>
std::pair<future<T>, promise<T>> create_future_promise_pair() noexcept
{
    future_promise_control_block<T>* control_block = new future_promise_control_block<T>();
    future<T> fut(control_block);
    promise<T> prom(control_block);
    return std::pair<future<T>, promise<T>>(std::move(fut), std::move(prom));
}

template <typename Arg>
template <typename Ret>
future<Ret> future<Arg>::then(std::function<Ret(con::result<Arg>)> corutine)
{
    auto [fut, prom] = create_future_promise_pair<Ret>();

    control_block_->continuation
        = [prom = std::move(prom), corutine = std::move(corutine)](con::result<Arg> arg) mutable -> unit {
        try {
            prom.set_value(corutine(std::move(arg)));
        } catch (...) {
            prom.set_exception(std::current_exception());
            return unit {};
        }
        return unit {};
    };

    return std::move(fut);
}

} // namespace co
