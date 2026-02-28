#include "result.hpp"
#include "unittest.hpp"

#include "event_loop.hpp"

SIMPLE_TEST(event_loop_test_1)
{
    co::ev_loop loop;

    int iters = 0;

    loop.start([&]() {
        iters++;
        loop.invoke([&]() {
                iters++;
                return co::unit {};
            })
            .then([&](con::result<co::unit>) {
                loop.stop();
                iters++;
                return co::unit {};
            });
    });

    ASSERT_EQ(iters, 3);
}

TEST_MAIN()
