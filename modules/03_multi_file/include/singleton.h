#pragma once

// Meyers' Singleton —— 利用函数内 static 局部变量实现。
// C++11 起保证线程安全初始化（magic statics）。
// 避免了全局变量跨 TU 初始化顺序不确定的问题。

#include <string>

class Logger {
public:
    static Logger& instance();

    void setPrefix(std::string const& prefix);
    [[nodiscard]] std::string const& prefix() const;
    [[nodiscard]] int logCount() const;
    void log(std::string const& message);

    Logger(Logger const&) = delete;
    Logger& operator=(Logger const&) = delete;

private:
    Logger() = default;
    std::string prefix_{"[LOG]"};
    int log_count_{0};
};
