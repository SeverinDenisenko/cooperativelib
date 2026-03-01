#pragma once

#include <concepts>
#include <functional>
#include <type_traits>

namespace co {

template <typename Ret, typename... Args>
class move_only_function {
private:
    class functor_base {
    public:
        virtual Ret call(Args...) = 0;

        virtual ~functor_base() = default;
    };

    template <typename F>
    class functor : public functor_base {
    public:
        functor(F&& f)
            : functor_(std::move(f))
        {
        }

        Ret call(Args... args) override
        {
            return functor_(std::forward<Args>(args)...);
        }

    private:
        F functor_;
    };

    functor_base* function_;

public:
    template <typename F>
        requires(std::invocable<F&, Args...> && !std::same_as<std::decay_t<F>, move_only_function>)
    move_only_function(F&& f)
        : function_(new functor<std::decay_t<F>>(std::forward<F>(f)))
    {
    }

    move_only_function() noexcept
        : function_(nullptr)
    {
    }

    move_only_function(move_only_function&& other) noexcept
    {
        std::swap(other.function_, function_);
    }

    move_only_function& operator=(move_only_function&& other) noexcept
    {
        if (this == std::addressof(other)) {
            return *this;
        }

        std::swap(other.function_, function_);

        return *this;
    }

    template <typename F>
        requires(std::invocable<F&, Args...> && !std::same_as<std::decay_t<F>, move_only_function>)
    move_only_function& operator=(F&& f) noexcept
    {
        if (function_) {
            delete function_;
        }

        function_ = new functor<std::decay_t<F>>(std::forward<F>(f));

        return *this;
    }

    move_only_function(const move_only_function& other)            = delete;
    move_only_function& operator=(const move_only_function& other) = delete;

    ~move_only_function()
    {
        if (function_) {
            delete function_;
        }
    }

    Ret operator()(Args... args)
    {
        if (!function_) {
            throw std::bad_function_call();
        }

        return function_->call(std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept
    {
        return function_ != nullptr;
    }
};

template <typename... Args>
class move_only_function<void, Args...> {
private:
    class functor_base {
    public:
        virtual void call(Args...) = 0;

        virtual ~functor_base() = default;
    };

    template <typename F>
    class functor : public functor_base {
    public:
        functor(F&& f)
            : functor_(std::move(f))
        {
        }

        void call(Args... args) override
        {
            functor_(std::forward<Args>(args)...);
        }

    private:
        F functor_;
    };

    functor_base* function_;

public:
    template <typename F>
        requires(std::invocable<F&, Args...> && !std::same_as<std::decay_t<F>, move_only_function>)
    move_only_function(F&& f)
        : function_(new functor<std::decay_t<F>>(std::forward<F>(f)))
    {
    }

    move_only_function() noexcept
        : function_(nullptr)
    {
    }

    move_only_function(move_only_function&& other) noexcept
    {
        std::swap(other.function_, function_);
    }

    move_only_function& operator=(move_only_function&& other) noexcept
    {
        if (this == std::addressof(other)) {
            return *this;
        }

        std::swap(other.function_, function_);

        return *this;
    }

    template <typename F>
        requires(std::invocable<F&, Args...> && !std::same_as<std::decay_t<F>, move_only_function>)
    move_only_function& operator=(F&& f) noexcept
    {
        if (function_) {
            delete function_;
        }

        function_ = new functor<std::decay_t<F>>(std::forward<F>(f));

        return *this;
    }

    move_only_function(const move_only_function& other)            = delete;
    move_only_function& operator=(const move_only_function& other) = delete;

    ~move_only_function()
    {
        if (function_) {
            delete function_;
        }
    }

    void operator()(Args... args)
    {
        if (!function_) {
            throw std::bad_function_call();
        }

        function_->call(std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept
    {
        return function_ != nullptr;
    }
};

}
