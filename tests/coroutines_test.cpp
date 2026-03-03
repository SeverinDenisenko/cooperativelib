#include "coroutine.hpp"
#include "unittest.hpp"

#include <exception>
#include <stdexcept>

inline int calls = 0;

co::coroutine<int> test_coroutine_1()
{
    calls += 1;
    co_return 1;
}

co::coroutine<int> test_coroutine_2()
{
    calls += 1;
    int a = co_await test_coroutine_1();
    ASSERT_EQ(a, 1);
    co_return a + 1;
}

co::coroutine<void> test_coroutine_3()
{
    calls += 1;
    int a = co_await test_coroutine_1();
    ASSERT_EQ(a, 1);
    int b = co_await test_coroutine_2();
    ASSERT_EQ(b, 2);
}

co::coroutine<void> test_coroutine_4()
{
    throw std::runtime_error("test_coroutine_4");
}

co::coroutine<void> test_coroutine_5()
{
    co_await test_coroutine_4();
}

SIMPLE_TEST(coroutine_test_1)
{
    calls                   = 0;
    co::coroutine<int> coro = test_coroutine_1();
    ASSERT_EQ(calls, 1);
    ASSERT_TRUE(coro.done());
    ASSERT_EQ(coro.get(), 1);
}

SIMPLE_TEST(coroutine_test_2)
{
    calls                     = 0;
    co::coroutine<int> result = test_coroutine_2();
    ASSERT_TRUE(result.done());
    ASSERT_EQ(result.get(), 2);
    ASSERT_EQ(calls, 2);
}

SIMPLE_TEST(coroutine_test_3)
{
    calls                    = 0;
    co::coroutine<void> coro = test_coroutine_3();
    ASSERT_TRUE(coro.done());
    ASSERT_EQ(calls, 4);
}

SIMPLE_TEST(coroutine_test_4)
{
    co::coroutine<void> coro = test_coroutine_5();

    ASSERT_TRUE(coro.done());

    try {
        coro.get();
        ASSERT_TRUE(false);
    } catch (const std::exception& ex) {
        ASSERT_EQ(std::string(ex.what()), "test_coroutine_4");
    }
}

TEST_MAIN()
