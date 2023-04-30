cmake_minimum_required(VERSION 3.25)
project(cpp_utils)

set(CMAKE_CXX_STANDARD 11)

# 引入 conan
include(${CMAKE_BINARY_DIR}/i)
conan_basic_setup()

# 添加测试
add_executable(cpp_utils_test
        common/fs_utils.h
        test/main_test.cpp)

# 链接 GTest 库
find_package(GTest REQUIRED)
target_link_libraries(cpp_utils_test GTest::GTest GTest::Main)

