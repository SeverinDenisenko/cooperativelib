#pragma once

#include "future.hpp"

#include <coroutine>
#include <memory>

namespace co {

template <typename T>
class awaiter;

template <typename T>
class corutine {
public:
    class promise_type {
    public:
        promise_type()
            : promise_(create_promise<T>())
        {
        }

        corutine get_return_object()
        {
            return corutine { promise_.get_future(), std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_never initial_suspend()
        {
            return {};
        }

        std::suspend_always final_suspend() noexcept
        {
            return {};
        }

        void return_value(T value)
        {
            promise_.set_value(value);
        }

        void unhandled_exception()
        {
            promise_.set_exception(std::current_exception());
        }

    private:
        promise<T> promise_;
    };

    corutine(future<T>&& future, std::coroutine_handle<promise_type> handle)
        : future_(std::move(future))
        , handle_(handle)
    {
    }

    corutine(const corutine&)            = delete;
    corutine& operator=(const corutine&) = delete;

    corutine(corutine&& other) noexcept
    {
        future_       = std::move(other.future_);
        handle_       = other.handle_;
        other.handle_ = nullptr;
    }

    corutine& operator=(corutine&& other) noexcept
    {
        if (this == std::addressof(other)) {
            return *this;
        }

        if (handle_) {
            handle_.destroy();
        }

        future_       = std::move(other.future_);
        handle_       = other.handle_;
        other.handle_ = nullptr;

        return *this;
    }

    ~corutine()
    {
        if (handle_) {
            handle_.destroy();
        }
    }

    auto operator co_await()
    {
        return awaiter<T> { std::move(future_) };
    }

    future<T> get_future()
    {
        return std::move(future_);
    }

private:
    future<T> future_;
    std::coroutine_handle<promise_type> handle_;
};

template <typename T>
class awaiter {
public:
    awaiter(future<T>&& future)
        : future_(std::move(future))
    {
    }

    bool await_ready() const noexcept
    {
        return future_.ready();
    }

    void await_suspend(std::coroutine_handle<> handle)
    {
        future_.then([handle]([[maybe_unused]] con::result<T> result) mutable -> std::monostate {
            handle.resume();
            return std::monostate {};
        });
    }

    T await_resume()
    {
        return future_.get();
    }

private:
    future<T> future_;
};

}
