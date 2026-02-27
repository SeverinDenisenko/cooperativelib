#pragma once

#include "future.hpp"

#include <coroutine>

namespace co {

template <typename T>
class awaiter;

template <typename T>
class task {
public:
    struct promise_type {
        promise<T> promise;

        task get_return_object()
        {
            return task { promise.get_future() };
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
            promise.set_value(value);
        }

        void unhandled_exception()
        {
            promise.set_exception(std::current_exception());
        }
    };

    task(future<T>&& future)
        : future_(std::move(future))
    {
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
};

template <typename T>
class awaiter {
public:
    awaiter(future<T>&& future)
        : future(std::move(future))
    {
    }

    bool await_ready() const noexcept
    {
        return future.ready();
    }

    void await_suspend(std::coroutine_handle<> handle)
    {
        future.template then<std::monostate>([handle](con::result<T> _) mutable -> std::monostate {
            handle.resume();
            return std::monostate {};
        });
    }

    T await_resume()
    {
        return future.get();
    }

private:
    future<T> future;
};

}
