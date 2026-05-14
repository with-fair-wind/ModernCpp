#pragma once

// 前置声明演示：B 持有 A 的指针，无需 #include "forward_a.h"。

#include <string>

class NodeA;  // 前置声明

class NodeB {
public:
    explicit NodeB(std::string name);
    ~NodeB();

    void setPartner(NodeA* a);
    [[nodiscard]] const std::string& name() const {
        return name_;
    }
    [[nodiscard]] std::string describe() const;

private:
    std::string name_;
    NodeA* partner_{nullptr};
};
