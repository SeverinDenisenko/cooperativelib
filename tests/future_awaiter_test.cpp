#include "result.hpp"
#include "unittest.hpp"

#include "coroutine.hpp"
#include "future.hpp"
#include "future_awaiter.hpp"

co::coroutine<int> test_coroutine_1(co::future<int> future)
{
    int result = co_await co::future_awaiter<int> { std::move(future) };
    ASSERT_EQ(result, 43);
    co_return result;
}

co::coroutine<int> test_coroutine_2()
{
    co_return 42;
}

SIMPLE_TEST(future_awaiter_test_1)
{
    auto [fut, prom] = co::create_future_promise<int>();

    auto fut2 = std::move(fut).then([](con::result<int> res) {
        ASSERT_EQ(res.value(), 42);
        return res.value() + 1;
    });

    co::coroutine<int> coro = test_coroutine_1(std::move(fut2));

    ASSERT_FALSE(coro.done());

    prom.set_value(42);

    ASSERT_TRUE(coro.done());
    ASSERT_EQ(coro.get(), 43);
}

SIMPLE_TEST(future_awaiter_test_2)
{
    auto [fut, prom] = co::create_future_promise<int>();

    prom.set_value(42);

    auto fut2 = std::move(fut).then([](con::result<int> res) {
        ASSERT_EQ(res.value(), 42);
        return res.value() + 1;
    });

    co::coroutine<int> coro = test_coroutine_1(std::move(fut2));

    ASSERT_TRUE(coro.done());
    ASSERT_EQ(coro.get(), 43);
}

TEST_MAIN()
