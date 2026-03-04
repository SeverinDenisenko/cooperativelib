#include "coroutine.hpp"
#include "event_loop.hpp"
#include "function.hpp"
#include "future_awaiter.hpp"
#include "result.hpp"
#include "unittest.hpp"

inline int iters = 0;

class event_loop_awaiter {
public:
    event_loop_awaiter(co::ev_loop& ev_loop, co::move_only_function<void> callback)
        : ev_loop_ { ev_loop }
        , callback_ { std::move(callback) }
    {
    }

    bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> calling)
    {
        ev_loop_.post([calling, this]() {
            callback_();
            calling.resume();
        });
    }

    void await_resume()
    {
    }

private:
    co::ev_loop& ev_loop_;
    co::move_only_function<void> callback_;
};

co::coroutine<void> skip_ev_loop(co::ev_loop& loop, co::move_only_function<void> callback)
{
    iters++;
    co_await co::future_awaiter { loop.invoke([callback = std::move(callback)]() {
        iters++;
        callback();
        return con::unit { };
    }) };
}

co::coroutine<void> skip_ev_loop2(co::ev_loop& loop, co::move_only_function<void> callback)
{
    iters++;
    co_await event_loop_awaiter { loop, [callback = std::move(callback)]() {
                                     iters++;
                                     callback();
                                 } };
}

SIMPLE_TEST(event_loop_test_1)
{
    co::ev_loop loop;

    iters = 0;

    co::coroutine<void> coro { };

    loop.post([&]() {
        iters++;
        coro = skip_ev_loop(loop, [&]() { loop.stop(); });
    });

    loop.start();

    ASSERT_EQ(iters, 3);
}

SIMPLE_TEST(event_loop_test_2)
{
    co::ev_loop loop;

    iters = 0;

    co::coroutine<void> coro { };

    loop.post([&]() {
        iters++;
        coro = skip_ev_loop2(loop, [&]() { loop.stop(); });
    });

    loop.start();

    ASSERT_EQ(iters, 3);
}

TEST_MAIN()
