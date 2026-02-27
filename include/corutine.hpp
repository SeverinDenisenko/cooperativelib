#pragma once

#include "future.hpp"

#include <coroutine>
#include <memory>

namespace co {

template <typename T>
class awaiter;

template <typename T>
class task {
public:
    class promise_type {
    public:
        task get_return_object()
        {
            return task { promise_.get_future(), std::coroutine_handle<promise_type>::from_promise(*this) };
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

    task(future<T>&& future, std::coroutine_handle<promise_type> handle)
        : future_(std::move(future))
        , handle_(handle)
    {
    }

    task(const task&)            = delete;
    task& operator=(const task&) = delete;

    task(task&& other) noexcept
    {
        future_       = std::move(other.future_);
        handle_       = other.handle_;
        other.handle_ = nullptr;
    }

    task& operator=(task&& other) noexcept
    {
        if (this == std::addressof(other)) {
            return *this;
        }

        if (handle_ && !handle_.done()) {
            handle_.destroy();
        }

        future_       = std::move(other.future_);
        handle_       = other.handle_;
        other.handle_ = nullptr;

        return *this;
    }

    ~task()
    {
        if (handle_ && !handle_.done()) {
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
        future_.template then<std::monostate>(
            [handle]([[maybe_unused]] con::result<T> result) mutable -> std::monostate {
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
