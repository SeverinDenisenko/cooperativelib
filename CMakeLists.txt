cmake_minimum_required(VERSION 3.10)

project(cooperative)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

option(ENABLE_TESTING OFF)

set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/containerslib)

add_library(cooperative INTERFACE)

target_include_directories(cooperative 
    INTERFACE 
        ${PROJECT_INCLUDE_DIR}
)

target_link_libraries(cooperative
    INTERFACE
        containers
)

if (ENABLE_TESTING)
    enable_testing()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/unittestlib)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()
