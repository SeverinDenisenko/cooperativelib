#pragma once

#include "error.hpp"
#include "result.hpp"

#include <coroutine>
#include <cstddef>
#include <exception>
#include <memory>

namespace co {

template <typename T = void>
class coroutine {
public:
    struct final_awaiter {
        bool await_ready() const noexcept
        {
            return false;
        }

        template <typename P>
        auto await_suspend(std::coroutine_handle<P> handle) noexcept
        {
            return handle.promise().continuation;
        }

        void await_resume() const noexcept
        {
        }
    };

    struct promise {
        std::coroutine_handle<> continuation { std::noop_coroutine() };
        con::result<T> result { };

        void unhandled_exception() noexcept
        {
            result = std::current_exception();
        }

        std::suspend_never initial_suspend() noexcept
        {
            return { };
        }

        final_awaiter final_suspend() noexcept
        {
            return { };
        }

        coroutine get_return_object()
        {
            return coroutine { std::coroutine_handle<promise>::from_promise(*this) };
        }

        void return_value(T&& res) noexcept
        {
            result = std::move(res);
        }
    };

    struct awaiter {
        std::coroutine_handle<promise> handle;

        bool await_ready() const noexcept
        {
            return !handle || handle.done();
        }

        std::coroutine_handle<promise> await_suspend(std::coroutine_handle<> calling) noexcept
        {
            handle.promise().continuation = calling;
            return handle;
        }

        template <typename U = T>
            requires(std::is_same_v<U, void>)
        void await_resume()
        {
            handle.promise().result.value();
        }

        template <typename U = T>
            requires(!std::is_same_v<U, void>)
        T await_resume()
        {
            return std::move(handle.promise().result.value());
        }
    };

    using promise_type = promise;

    ~coroutine()
    {
        if (handle_) {
            handle_.destroy();
        }
    }

    coroutine()
        : handle_(nullptr)
    {
    }

    coroutine(const coroutine& other)            = delete;
    coroutine& operator=(const coroutine& other) = delete;

    coroutine(coroutine&& other)
    {
        std::swap(handle_, other.handle_);
    }

    coroutine& operator=(coroutine&& other)
    {
        if (this == std::addressof(other)) {
            return *this;
        }

        std::swap(handle_, other.handle_);

        return *this;
    }

    auto operator co_await()
    {
        if (!handle_) {
            throw con::error("empty coroutine");
        }

        return awaiter { handle_ };
    }

    bool done()
    {
        if (!handle_) {
            throw con::error("empty coroutine");
        }

        return handle_.done();
    }

    template <typename U = T>
        requires(std::is_same_v<U, void>)
    void get()
    {
        if (!done()) {
            throw con::error("coroutine is not ready");
        }

        if (!handle_) {
            throw con::error("empty coroutine");
        }

        handle_.promise().result.value();
    }

    template <typename U = T>
        requires(!std::is_same_v<U, void>)
    T get()
    {
        if (!done()) {
            throw con::error("coroutine is not ready");
        }

        if (!handle_) {
            throw con::error("empty coroutine");
        }

        return handle_.promise().result.value();
    }

private:
    explicit coroutine(std::coroutine_handle<promise> handle)
        : handle_(handle)
    {
    }

    std::coroutine_handle<promise> handle_ { };
};

template <>
struct coroutine<void>::promise {
    std::coroutine_handle<> continuation { std::noop_coroutine() };
    con::result<con::unit> result { };

    void unhandled_exception() noexcept
    {
        result = std::current_exception();
    }

    std::suspend_never initial_suspend() noexcept
    {
        return { };
    }

    final_awaiter final_suspend() noexcept
    {
        return { };
    }

    coroutine get_return_object()
    {
        return coroutine { std::coroutine_handle<promise>::from_promise(*this) };
    }

    void return_void() noexcept
    {
        result = con::unit { };
    }
};

}
