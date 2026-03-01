#include "future.hpp"
#include "result.hpp"
#include "unittest.hpp"

#include "event_loop.hpp"

#include <thread>

SIMPLE_TEST(event_loop_test_1)
{
    co::ev_loop loop;

    int iters = 0;

    loop.post([&]() {
        iters++;
        loop.invoke([&]() {
                iters++;
                return co::unit {};
            })
            .then([&](con::result<co::unit>) {
                iters++;
                loop.stop();
                return co::unit {};
            });
    });

    loop.start();

    ASSERT_EQ(iters, 3);
}

SIMPLE_TEST(event_loop_test_2)
{
    co::ev_loop loop_1;
    co::ev_loop loop_2;

    std::thread thread_1([&loop_1, &loop_2]() {
        int answer = 0;

        loop_1.post([&]() {
            loop_1
                .invoke(
                    loop_2,
                    [&]() {
                        loop_2.stop();
                        return 42;
                    })
                .then([&](con::result<int> result) {
                    loop_1.stop();
                    answer = result.value();
                    return co::unit {};
                });
        });

        loop_1.start();

        ASSERT_EQ(answer, 42);
    });

    std::thread thread_2([&loop_2]() { loop_2.start(); });

    thread_1.join();
    thread_2.join();
}

TEST_MAIN()
