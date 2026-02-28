#include "corutine.hpp"
#include "event_loop.hpp"
#include "unittest.hpp"

void wait_for_loops(uint32_t loops, co::ev_loop& loop, co::promise<co::unit> promise)
{
    if (loops > 0) {
        --loops;
        loop.post([loops, &loop, promise = std::move(promise)]() mutable {
            wait_for_loops(loops, loop, std::move(promise));
        });
    } else {
        promise.set_value(co::unit {});
    }
}

co::awaiter<co::unit> wait_for(co::ev_loop& loop, uint32_t loops)
{
    auto [fut, prom] = co::create_future_promise_pair<co::unit>();

    wait_for_loops(loops, loop, std::move(prom));

    return co::awaiter<co::unit>(std::move(fut));
}

co::corutine<int> test_corutine_1()
{
    co_return 1;
}

co::corutine<int> test_corutine_2()
{
    co_return co_await test_corutine_1() + 1;
}

co::corutine<int> test_corutine_3()
{
    co_return co_await test_corutine_1() + co_await test_corutine_2();
}

co::corutine<int> test_corutine_4(co::ev_loop& loop)
{
    co_await wait_for(loop, 10);
    co_return 42;
}

SIMPLE_TEST(corutine_test_1)
{
    co::corutine<int> result = test_corutine_1();
    ASSERT_EQ(result.get_future().get(), 1);
}

SIMPLE_TEST(corutine_test_2)
{
    co::corutine<int> result = test_corutine_2();
    ASSERT_EQ(result.get_future().get(), 2);
}

SIMPLE_TEST(corutine_test_3)
{
    co::corutine<int> result = test_corutine_3();
    ASSERT_EQ(result.get_future().get(), 3);
}

SIMPLE_TEST(corutine_test_4)
{
    co::corutine<int> result  = test_corutine_1();
    co::corutine<int> result2 = std::move(result);
    ASSERT_EQ(result2.get_future().get(), 1);
}

SIMPLE_TEST(corutine_test_5)
{
    co::ev_loop loop;
    co::corutine<int> result = test_corutine_4(loop);
    loop.start([]() { });
    ASSERT_EQ(result.get_future().get(), 42);
}

TEST_MAIN()
