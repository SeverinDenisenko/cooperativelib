#pragma once

#include <functional>
#include <list>

#include "future.hpp"

namespace co {

class ev_loop {
public:
    using task_t = std::function<void(void)>;

    ev_loop() = default;

    ev_loop(const ev_loop&)            = delete;
    ev_loop& operator=(const ev_loop&) = delete;

    void start(task_t corutine)
    {
        stop_ = false;
        corutine();

        while (!stop_ && !task_queue_.empty()) {
            task_queue_.front()();
            task_queue_.pop_front();
        }
    }

    void push(task_t corutine)
    {
        task_queue_.push_back(std::move(corutine));
    }

    template <typename Ret>
    future<Ret> push(std::function<Ret(void)> corutine)
    {
        auto [fut, prom] = create_future_promise_pair<Ret>();
        task_queue_.push_back([corutine = std::move(corutine), prom = std::move(prom)]() mutable {
            try {
                prom.set_value(corutine());
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
