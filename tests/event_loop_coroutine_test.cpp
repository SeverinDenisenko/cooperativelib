#include "coroutine.hpp"
#include "event_loop.hpp"
#include "function.hpp"
#include "future_awaiter.hpp"
#include "result.hpp"
#include "unittest.hpp"

inline int iters = 0;

co::coroutine<void> skip_ev_loop(co::ev_loop& loop, co::move_only_function<void> callback)
{
    iters++;
    co_await co::future_awaiter { loop.invoke([callback = std::move(callback)]() {
        iters++;
        callback();
        return con::unit { };
    }) };
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

TEST_MAIN()
