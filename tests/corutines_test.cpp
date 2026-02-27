#include "corutine.hpp"
#include "unittest.hpp"

co::task<int> test_corutine_1()
{
    co_return 1;
}

co::task<int> test_corutine_2()
{
    co_return co_await test_corutine_1() + 1;
}

co::task<int> test_corutine_3()
{
    co_return co_await test_corutine_1() + co_await test_corutine_2();
}

SIMPLE_TEST(corutine_test_1)
{
    co::task<int> result = test_corutine_1();
    ASSERT_EQ(result.get_future().get(), 1);
}

SIMPLE_TEST(corutine_test_2)
{
    co::task<int> result = test_corutine_2();
    ASSERT_EQ(result.get_future().get(), 2);
}

SIMPLE_TEST(corutine_test_3)
{
    co::task<int> result = test_corutine_3();
    ASSERT_EQ(result.get_future().get(), 3);
}

TEST_MAIN()
