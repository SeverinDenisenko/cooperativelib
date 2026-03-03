#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>

namespace co {

template <typename Ret, typename... Args>
class move_only_function {
private:
    class functor_base {
    public:
        virtual std::conditional_t<std::is_same_v<Ret, void>, void, Ret> call(Args...) const = 0;

        virtual ~functor_base() = default;
    };

    template <typename F>
    class functor : public functor_base {
    public:
        functor(F&& f)
            : functor_(std::move(f))
        {
        }

        std::conditional_t<std::is_same_v<Ret, void>, void, Ret> call(Args... args) const override
        {
            return functor_(std::forward<Args>(args)...);
        }

    private:
        mutable F functor_;
    };

    std::unique_ptr<functor_base> function_ { };

public:
    template <typename F>
        requires(std::invocable<F&, Args...> && !std::same_as<std::decay_t<F>, move_only_function>)
    move_only_function(F&& f)
        : function_(std::make_unique<functor<std::decay_t<F>>>(std::forward<F>(f)))
    {
    }

    move_only_function() noexcept
        : function_(nullptr)
    {
    }

    move_only_function(move_only_function&& other) noexcept            = default;
    move_only_function& operator=(move_only_function&& other) noexcept = default;

    template <typename F>
        requires(std::invocable<F&, Args...> && !std::same_as<std::decay_t<F>, move_only_function>)
    move_only_function& operator=(F&& f) noexcept
    {
        function_ = std::make_unique<functor<std::decay_t<F>>>(std::forward<F>(f));

        return *this;
    }

    move_only_function(const move_only_function& other)            = delete;
    move_only_function& operator=(const move_only_function& other) = delete;

    ~move_only_function() = default;

    std::conditional_t<std::is_same_v<Ret, void>, void, Ret> operator()(Args... args) const
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

}
