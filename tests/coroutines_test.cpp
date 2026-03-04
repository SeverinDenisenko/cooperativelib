#include "coroutine.hpp"
#include "unittest.hpp"

#include <exception>
#include <stdexcept>

inline int calls = 0;

co::coroutine<int> increment_calls_1_return_1()
{
    calls += 1;
    co_return 1;
}

co::coroutine<int> increment_calls_1_return_2()
{
    calls += 1;
    int a = co_await increment_calls_1_return_1();
    ASSERT_EQ(a, 1);
    co_return a + 1;
}

co::coroutine<void> increment_calls_1_return_void()
{
    calls += 1;
    int a = co_await increment_calls_1_return_1();
    ASSERT_EQ(a, 1);
    int b = co_await increment_calls_1_return_2();
    ASSERT_EQ(b, 2);
}

co::coroutine<void> throw_exception()
{
    throw std::runtime_error("throw_exception");
}

co::coroutine<void> test_coroutine_2()
{
    co_await throw_exception();
}

SIMPLE_TEST(coroutine_test_1)
{
    calls                   = 0;
    co::coroutine<int> coro = increment_calls_1_return_1();
    ASSERT_EQ(calls, 1);
    ASSERT_TRUE(coro.done());
    ASSERT_EQ(coro.get(), 1);
}

SIMPLE_TEST(coroutine_test_2)
{
    calls                     = 0;
    co::coroutine<int> result = increment_calls_1_return_2();
    ASSERT_TRUE(result.done());
    ASSERT_EQ(result.get(), 2);
    ASSERT_EQ(calls, 2);
}

SIMPLE_TEST(coroutine_test_3)
{
    calls                    = 0;
    co::coroutine<void> coro = increment_calls_1_return_void();
    ASSERT_TRUE(coro.done());
    ASSERT_EQ(calls, 4);
}

SIMPLE_TEST(exception_test)
{
    co::coroutine<void> coro = test_coroutine_2();

    ASSERT_TRUE(coro.done());

    try {
        coro.get();
        ASSERT_TRUE(false);
    } catch (const std::exception& ex) {
        ASSERT_EQ(std::string(ex.what()), "throw_exception");
    }
}

TEST_MAIN()
