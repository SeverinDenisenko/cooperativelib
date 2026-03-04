#include "result.hpp"
#include "unittest.hpp"

#include "future.hpp"

SIMPLE_TEST(unresolved_future_test)
{
    auto [fut, prom] = co::create_future_promise<int>();

    ASSERT_TRUE(!fut.ready());
    ASSERT_TRUE(!fut.has_value());
    ASSERT_TRUE(!fut.has_exception());
}

SIMPLE_TEST(resolved_future_test)
{
    auto [fut, prom] = co::create_future_promise<int>();

    prom.set_value(1);

    ASSERT_TRUE(fut.ready());
    ASSERT_TRUE(fut.has_value());
    ASSERT_TRUE(!fut.has_exception());
    ASSERT_EQ(fut.get(), 1);
}

SIMPLE_TEST(exception_future_test)
{
    auto [fut, prom] = co::create_future_promise<int>();

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

SIMPLE_TEST(exception_future_continuation_test)
{
    auto [fut, prom] = co::create_future_promise<int>();

    co::future<int> cont = std::move(fut).then([](con::result<int> i) { return i.value() + 1; });

    try {
        throw std::runtime_error("test");
    } catch (...) {
        prom.set_exception(std::current_exception());
    }

    ASSERT_TRUE(cont.ready());
    ASSERT_TRUE(!cont.has_value());
    ASSERT_TRUE(cont.has_exception());

    try {
        cont.get();
        ASSERT_TRUE(false);
    } catch (const std::runtime_error& e) {
        ASSERT_EQ(std::string(e.what()), "test");
    }
}

SIMPLE_TEST(future_continuation_test)
{
    auto [fut, prom] = co::create_future_promise<int>();

    co::future<int> cont = std::move(fut).then([](con::result<int> i) { return i.value() + 1; });

    prom.set_value(1);

    ASSERT_TRUE(cont.ready());
    ASSERT_TRUE(cont.has_value());
    ASSERT_TRUE(!cont.has_exception());

    ASSERT_EQ(cont.get(), 2);
}

SIMPLE_TEST(future_multiple_continuation_test)
{
    auto [fut, prom] = co::create_future_promise<int>();

    co::future<int> cont = std::move(fut)
                               .then([](con::result<int> i) {
                                   return i.value() + 1; //
                               })
                               .then([](con::result<int> i) {
                                   return i.value() + 1; //
                               });

    prom.set_value(1);

    ASSERT_TRUE(cont.ready());
    ASSERT_TRUE(cont.has_value());
    ASSERT_TRUE(!cont.has_exception());

    ASSERT_EQ(cont.get(), 3);
}

SIMPLE_TEST(future_moved_test)
{
    auto [fut, prom] = co::create_future_promise<int>();

    {
        co::promise<int> prom2;
        prom2 = std::move(prom);
    }

    try {
        fut.get();
        ASSERT_TRUE(false);
    } catch (...) {
    }
}

SIMPLE_TEST(future_continuation_after_resolved_test)
{
    auto [fut, prom] = co::create_future_promise<int>();

    prom.set_value(1);

    ASSERT_TRUE(fut.ready());
    ASSERT_TRUE(fut.has_value());
    ASSERT_TRUE(!fut.has_exception());
    ASSERT_EQ(fut.get(), 1);

    auto fut2 = std::move(fut).then([](con::result<int> res) { return res.value() + 1; });

    ASSERT_TRUE(fut2.ready());
    ASSERT_TRUE(fut2.has_value());
    ASSERT_TRUE(!fut2.has_exception());
    ASSERT_EQ(fut2.get(), 2);
}

SIMPLE_TEST(future_unresolved_continuation_test)
{
    auto [f, p] = co::create_future_promise<int>();

    auto f2 = std::move(f).then([](con::result<int> res) { return res.value() + 1; });
}

TEST_MAIN()
