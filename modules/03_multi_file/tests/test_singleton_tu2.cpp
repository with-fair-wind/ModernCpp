// 单例测试第二个 TU —— 验证跨 TU 获取的是同一单例实例。

#include <string>

#include "singleton.h"

// 在本 TU 提供 Logger 的实现（test 可执行文件需要）
Logger& Logger::instance() {
    static Logger inst;
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
    (void)message;  // 测试中不输出
}

// 给主测试 TU 调用的接口
Logger* getSingletonAddrFromTu2() {
    return &Logger::instance();
}

void logFromTu2(std::string const& msg) {
    Logger::instance().log(msg);
}
