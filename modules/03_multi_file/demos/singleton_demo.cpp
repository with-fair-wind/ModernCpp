// Meyers' Singleton 演示。
// 利用函数内 static 局部变量保证：
//   1. 延迟初始化（首次调用时构造）
//   2. 线程安全（C++11 magic statics）
//   3. 回避跨 TU 全局变量初始化顺序不确定的问题

#include <iostream>
#include <string>

#include "singleton.h"

// 单例的实现
Logger& Logger::instance() {
    static Logger inst;  // 延迟初始化，线程安全
    return inst;
}

void Logger::setPrefix(std::string const& prefix) {
    prefix_ = prefix;
}

std::string const& Logger::prefix() const {
    return prefix_;
}

int Logger::logCount() const {
    return log_count_;
}

void Logger::log(std::string const& message) {
    ++log_count_;
    std::cout << prefix_ << " " << message << '\n';
}

// NOLINTNEXTLINE(bugprone-exception-escape)
int main() {
    std::cout << "===== Meyers' Singleton 演示 =====\n\n";

    // 获取单例引用
    Logger& logger1 = Logger::instance();
    Logger& logger2 = Logger::instance();

    // 验证是同一实例
    std::cout << "logger1 地址: " << &logger1 << '\n';
    std::cout << "logger2 地址: " << &logger2 << '\n';
    std::cout << "是同一实例: " << (&logger1 == &logger2 ? "是" : "否") << "\n\n";

    // 使用单例
    logger1.log("第一条日志");
    logger1.log("第二条日志");

    // 修改前缀 —— 通过另一个引用也能观察到
    logger2.setPrefix("[WARN]");
    logger1.log("第三条日志（前缀已变）");

    std::cout << "\n日志计数: " << Logger::instance().logCount() << '\n';

    return 0;
}
