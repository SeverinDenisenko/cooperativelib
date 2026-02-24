#include "result.hpp"
#include "unittest.hpp"

#include "event_loop.hpp"
#include <variant>

SIMPLE_TEST(event_loop_test_1)
{
    co::ev_loop loop;

    int iters = 0;

    loop.start([&]() {
        iters++;
        loop.push<std::monostate>([&]() -> std::monostate {
                iters++;
                return std::monostate {};
            })
            .then<std::monostate>([&](con::result<std::monostate>) -> std::monostate {
                loop.stop();
                iters++;
                return std::monostate {};
            });
    });

    ASSERT_EQ(iters, 3);
}

TEST_MAIN()
