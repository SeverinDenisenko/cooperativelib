add_executable(future-test future_test.cpp)
add_executable(event-loop-test event_loop_test.cpp)

add_test(NAME future-test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/future-test)
add_test(NAME event-loop-test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/event-loop-test)

target_link_libraries(future-test unittest cooperative)
target_link_libraries(event-loop-test unittest cooperative)

if(MSVC)
    target_compile_options(future-test PRIVATE /W4 /WX)
    target_compile_options(event-loop-test PRIVATE /W4 /WX)
else()
    target_compile_options(future-test PRIVATE -Wall -Wextra -Werror)
    target_compile_options(event-loop-test PRIVATE -Wall -Wextra -Werror)
endif()
