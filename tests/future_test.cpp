#include "unittest.hpp"

#include "future.hpp"
#include <exception>

SIMPLE_TEST(future_test_1)
{
    auto [fut, prom] = co::create_futue_promice_pair<int>();

    ASSERT_TRUE(!fut.ready());
    ASSERT_TRUE(!fut.has_value());
    ASSERT_TRUE(!fut.has_exception());
}

SIMPLE_TEST(future_test_2)
{
    auto [fut, prom] = co::create_futue_promice_pair<int>();

    prom.set_value(1);

    ASSERT_TRUE(fut.ready());
    ASSERT_TRUE(fut.has_value());
    ASSERT_TRUE(!fut.has_exception());
    ASSERT_EQ(fut.get(), 1);
}

SIMPLE_TEST(future_test_3)
{
    auto [fut, prom] = co::create_futue_promice_pair<int>();

    try {
        throw std::runtime_error("test");
    } catch (...) {
        prom.set_exception(std::current_exception());
    }

    ASSERT_TRUE(fut.ready());
    ASSERT_TRUE(!fut.has_value());
    ASSERT_TRUE(fut.has_exception());

    try {
        fut.get();
        ASSERT_TRUE(false);
    } catch (const std::runtime_error& e) {
        ASSERT_EQ(std::string(e.what()), "test");
    }
}

SIMPLE_TEST(future_test_4)
{
    auto [fut, prom] = co::create_futue_promice_pair<int>();

    co::future<int> cont = fut.then<int>([](con::result<int> i) { return i.value() + 1; });

    try {
        throw std::runtime_error("test");
    } catch (...) {
        prom.set_exception(std::current_exception());
    }

    ASSERT_TRUE(fut.ready());
    ASSERT_TRUE(!fut.has_value());
    ASSERT_TRUE(fut.has_exception());

    try {
        cont.get();
        ASSERT_TRUE(false);
    } catch (const std::runtime_error& e) {
        ASSERT_EQ(std::string(e.what()), "test");
    }
}

SIMPLE_TEST(future_test_5)
{
    auto [fut, prom] = co::create_futue_promice_pair<int>();

    co::future<int> cont = fut.then<int>([](con::result<int> i) { return i.value() + 1; });

    prom.set_value(1);

    ASSERT_TRUE(fut.ready());
    ASSERT_TRUE(fut.has_value());
    ASSERT_TRUE(!fut.has_exception());

    ASSERT_EQ(cont.get(), 2);
}

TEST_MAIN()
