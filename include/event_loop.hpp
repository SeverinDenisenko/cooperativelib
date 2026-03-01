#pragma once

#include <concepts>
#include <list>
#include <mutex>
#include <type_traits>

#include "function.hpp"
#include "future.hpp"
#include "result.hpp"

namespace co {

class ev_loop {
public:
    ev_loop() = default;

    ev_loop(const ev_loop&)            = delete;
    ev_loop& operator=(const ev_loop&) = delete;

    /*
        This function blocks thread until ev_loop is stopped
    */
    void start()
    {
        while (true) {
            move_only_function<void> task {};

            {
                std::unique_lock lock(mutex_);
                if (stop_) {
                    break;
                }
                if (!task_queue_.empty()) {
                    task = std::move(task_queue_.front());
                    task_queue_.pop_front();
                }
            }

            if (task) {
                task();
            }
        }
    }

    /*
        Put task to event loop without ability to get result. Can be called on any thread.
    */
    template <typename Function>
        requires std::invocable<Function>
    void post(Function function)
    {
        std::unique_lock lock(mutex_);
        task_queue_.push_back(std::move(function));
    }

    /*
        Put task to event loop and get future. Can be used only on event loop thread.
    */
    template <typename Function>
        requires std::invocable<Function>
    future<std::invoke_result_t<Function>> invoke(Function function)
    {
        auto [fut, prom] = create_future_promise<std::invoke_result_t<Function>>();
        post([function = std::move(function), prom = std::move(prom)]() mutable {
            try {
                prom.set_value(function());
            } catch (...) {
                prom.set_exception(std::current_exception());
            }
        });
        return std::move(fut);
    }

    /*
        Put task to other event loop and get result on this event loop. Can be used only on this event loop thread.
    */
    template <typename Function>
        requires std::invocable<Function>
    future<std::invoke_result_t<Function>> invoke(ev_loop& other_ev_loop, Function function)
    {
        auto [fut, prom] = create_future_promise<std::invoke_result_t<Function>>();

        other_ev_loop.post([function = std::move(function), prom = std::move(prom), this]() mutable {
            con::result<std::invoke_result_t<Function>> result;

            try {
                result = function();
            } catch (...) {
                result = std::current_exception();
            }

            post([result = std::move(result), prom = std::move(prom)]() mutable {
                prom.resolve(std::move(result)); //
            });
        });

        return std::move(fut);
    }

    void stop()
    {
        std::unique_lock lock(mutex_);
        stop_ = true;
    }

private:
    std::mutex mutex_ {};
    std::list<move_only_function<void>> task_queue_ {};
    bool stop_ { false };
};

}
