cmake_minimum_required(VERSION 3.25)
project(cpp_utils)

set(CMAKE_CXX_STANDARD 11)

find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIR})

include_directories(common)

# 添加测试
add_executable(cpp_utils_test
        common/fs_utils.h
        test/main_test.cpp)

# 链接 GTest 库
target_link_libraries(cpp_utils_test GTest::GTest GTest::Main)

