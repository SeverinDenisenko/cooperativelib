#pragma once

#include "future.hpp"
#include "result.hpp"

#include <coroutine>

namespace co {

template <typename T = void>
class future_awaiter {
public:
    future_awaiter(future<T> future)
        : future_(std::move(future))
    {
    }

    bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> calling) noexcept
    {
        std::move(future_).then([calling, this](con::result<T> result) {
            result_ = std::move(result);
            calling.resume();
            return con::unit { };
        });
    }

    template <typename U = T>
        requires(std::is_same_v<U, void>)
    void await_resume()
    {
        result_.value();
    }

    template <typename U = T>
        requires(!std::is_same_v<U, void>)
    T await_resume()
    {
        return result_.value();
    }

private:
    con::result<T> result_ { };
    future<T> future_ { };
};

}
