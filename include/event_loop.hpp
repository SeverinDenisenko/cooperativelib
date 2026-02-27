#pragma once

#include <functional>
#include <list>

#include "future.hpp"

namespace co {

class ev_loop {
public:
    using task_t = std::function<void(void)>;

    ev_loop() = default;

    void start(task_t task)
    {
        stop_ = false;
        task();

        while (!stop_ && !task_queue_.empty()) {
            task_queue_.front()();
            task_queue_.pop_front();
        }
    }

    template <typename Ret>
    future<Ret> push(std::function<Ret(void)> task)
    {
        auto [fut, prom] = create_future_promise_pair<Ret>();
        task_queue_.push_back([task = std::move(task), prom = std::move(prom)]() mutable {
            try {
                prom.set_value(task());
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
    std::list<task_t> task_queue_;
    bool stop_;
};

}
