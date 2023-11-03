# cpp_utils

这是一个 C++ 实用程序库，包含了一些常用的功能，如文件系统操作和打印工具。

## 安装

本项目使用 [CMake](https://cmake.org/) 进行构建。请确保你的系统中已经安装了 CMake。

```sh
# 克隆仓库
git clone https://github.com/zhangxianbing/cpp_utils.git

# 进入项目目录
cd cpp_utils

# 创建并进入构建目录
mkdir build && cd build

# 构建项目
cmake ..

# 编译项目
make
```

## 测试

本项目使用 Google Test 进行单元测试。测试代码位于 [`test/`](command:_github.copilot.openRelativePath?%5B%22test%2F%22%5D "test/") 目录下。

```sh
# 在构建目录下运行测试
./cpp_utils_test
```

## 许可证

本项目采用 MIT 许可证，更多信息请查看 [LICENSE](LICENSE) 文件。
