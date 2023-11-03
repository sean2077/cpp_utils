/// 打印相关辅助函数
#pragma once

#include <algorithm>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>

/// ====================================== 特定类型的 operator<< 重载函数 ====================================== ///

namespace
{
}

/// ====================================== 通用 operator<< 模板函数 ====================================== ///

/// Note: 要打印 vector, set, map 等容器，需要容器内的元素支持 operator<<

// 打印 vector 的模板函数
template <typename T> inline std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec);

// 打印 set 的模板函数
template <typename T> inline std::ostream &operator<<(std::ostream &os, const std::set<T> &set);

// 打印 map 的模板函数
template <typename K, typename V> inline std::ostream &operator<<(std::ostream &os, const std::map<K, V> &m);

// 打印 unordered_map 的模板函数
template <typename K, typename V> inline std::ostream &operator<<(std::ostream &os, const std::unordered_map<K, V> &m);

// 打印 pair 的模板函数
template <typename T1, typename T2> inline std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p);

// 打印 tuple 的模板函数
template <typename... Args> inline std::ostream &operator<<(std::ostream &os, const std::tuple<Args...> &t);

// 打印 list 的模板函数
template <typename T> inline std::ostream &operator<<(std::ostream &os, const std::list<T> &list);

/// Note: 为支持嵌套的 vector, set, map 等容器，必须声明和定义分开

template <typename T> inline std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec)
{
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::stringstream ss;
        if (ss << vec[i]) {
            os << ss.str();
        } else {
            auto &&ref = vec.at(i);
            os << &ref;
        }
        if (i != vec.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

template <typename T> inline std::ostream &operator<<(std::ostream &os, const std::set<T> &set)
{
    os << "(";
    for (auto &&v : set) {
        std::stringstream ss;
        if (ss << v) {
            os << ss.str();
        } else {
            os << &v;
        }
        if (v != *set.rbegin()) {
            os << ", ";
        }
    }
    os << ")";
    return os;
}

template <typename K, typename V> inline std::ostream &operator<<(std::ostream &os, const std::map<K, V> &m)
{
    os << "{";
    size_t first = 0;
    for (const auto &p : m) {
        // print key
        std::stringstream ss;
        if (ss << p.first) {
            os << ss.str();
        } else {
            os << &p.first;
        }
        os << ": ";
        // print value
        ss.str("");
        if (ss << p.second) {
            os << ss.str();
        } else {
            os << &p.second;
        }

        // 如果不是最后一个元素，打印逗号
        if (++first != m.size()) {
            os << ", ";
        }
    }
    os << "}";
    return os;
}

template <typename K, typename V> inline std::ostream &operator<<(std::ostream &os, const std::unordered_map<K, V> &m)
{
    os << "{";
    size_t first = 0;
    for (const auto &p : m) {
        // print key
        std::stringstream ss;
        if (ss << p.first) {
            os << ss.str();
        } else {
            os << &p.first;
        }
        os << ": ";
        // print value
        ss.str("");
        if (ss << p.second) {
            os << ss.str();
        } else {
            os << &p.second;
        }

        // 如果不是最后一个元素，打印逗号
        if (++first != m.size()) {
            os << ", ";
        }
    }
    os << "}";
    return os;
}

template <typename T1, typename T2> inline std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p)
{
    os << "(";
    std::stringstream ss;
    if (ss << p.first) {
        os << ss.str();
    } else {
        os << &p.first;
    }
    os << ", ";
    ss.str("");
    if (ss << p.second) {
        os << ss.str();
    } else {
        os << &p.second;
    }
    os << ")";
    return os;
}

#if __cplusplus >= 201703L
template <typename... Args> std::ostream &operator<<(std::ostream &os, const std::tuple<Args...> &t)
{
    os << "(";
    std::apply(
        [&os](const Args &...tupleArgs) {
            size_t n{ 0 };
            ((os << tupleArgs << (++n != sizeof...(Args) ? ", " : "")), ...);
        },
        t);
    os << ")";
    return os;
}
#else
// Helper function to print tuple (C++11 and later)
template <typename Tuple, size_t N> struct PrintTuple {
    static void print(std::ostream &os, const Tuple &t)
    {
        PrintTuple<Tuple, N - 1>::print(os, t);
        os << ", " << std::get<N - 1>(t);
    }
};

// Partial specialization for the base case to end the recursion
template <typename Tuple> struct PrintTuple<Tuple, 1> {
    static void print(std::ostream &os, const Tuple &t)
    {
        os << std::get<0>(t);
    }
};

// Overload for empty tuples
template <typename Tuple> struct PrintTuple<Tuple, 0> {
    static void print(std::ostream &os, const Tuple &t)
    {
        // No elements to print
    }
};

// The actual operator<< that uses PrintTuple
template <typename... Args> std::ostream &operator<<(std::ostream &os, const std::tuple<Args...> &t)
{
    os << "(";
    PrintTuple<decltype(t), sizeof...(Args)>::print(os, t);
    os << ")";
    return os;
}
#endif

template <typename T> inline std::ostream &operator<<(std::ostream &os, const std::list<T> &list)
{
    os << "[";
    for (auto &&v : list) {
        std::stringstream ss;
        if (ss << v) {
            os << ss.str();
        } else {
            os << &v;
        }
        if (v != *list.rbegin()) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

/// ====================================== others ====================================== ///

// 打印漂亮的 map
template <typename K, typename V> inline std::string pretty_map(const std::map<K, V> &m)
{
    std::ostringstream ss;
    int max_key_len = 0, max_value_len = 0;
    for (auto &&kv : m) {
        std::ostringstream tss;
        tss << kv.first;
        max_key_len = std::max(max_key_len, (int)tss.str().length());
        tss.clear();
        tss << kv.second;
        max_value_len = std::max(max_value_len, (int)tss.str().length());
    }
    for (auto &&kv : m) {
        ss << "    * " << std::setw(max_key_len) << kv.first << ": " << std::setw(max_value_len) << kv.second << std::endl;
    }
    return ss.str();
}
