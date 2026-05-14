#pragma once

// 前置声明演示：A 持有 B 的指针，无需 #include "forward_b.h"。
// 这样 B 的头文件改动不会导致包含本文件的 TU 重新编译。

#include <string>

class NodeB;  // 前置声明 —— 不完整类型，只能用指针/引用

class NodeA {
public:
    explicit NodeA(std::string name);
    ~NodeA();

    void setPartner(NodeB* b);
    [[nodiscard]] const std::string& name() const {
        return name_;
    }
    [[nodiscard]] std::string describe() const;

private:
    std::string name_;
    NodeB* partner_{nullptr};  // 指针成员：不完整类型合法
};
