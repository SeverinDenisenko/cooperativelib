#pragma once

#include <concepts>
#include <list>
#include <type_traits>

#include "function.hpp"
#include "future.hpp"

namespace co {

class ev_loop {
public:
    ev_loop() = default;

    ev_loop(const ev_loop&)            = delete;
    ev_loop& operator=(const ev_loop&) = delete;

    template <typename Function>
        requires std::invocable<Function>
    void start(Function function)
    {
        stop_ = false;
        function();

        while (!stop_ && !task_queue_.empty()) {
            task_queue_.front()();
            task_queue_.pop_front();
        }
    }

    template <typename Function>
        requires std::invocable<Function>
    void post(Function function)
    {
        task_queue_.push_back(std::move(function));
    }

    template <typename Function>
        requires std::invocable<Function>
    future<std::invoke_result_t<Function>> invoke(Function function)
    {
        auto [fut, prom] = create_future_promise_pair<std::invoke_result_t<Function>>();
        post([function = std::move(function), prom = std::move(prom)]() mutable {
            try {
                prom.set_value(function());
            } catch (...) {
                prom.set_exception(std::current_exception());
            }
        });
        return std::move(fut);
    }

    void stop()
    {
        stop_ = true;
    }

private:
    std::list<move_only_function<void>> task_queue_;
    bool stop_;
};

}
