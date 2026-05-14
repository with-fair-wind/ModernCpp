// 前置声明测试的 NodeA 实现 TU。
// 内容与 demos/forward_a.cpp 相同，独立编译避免链接冲突。

#include <string>
#include <utility>

#include "forward_a.h"
#include "forward_b.h"

NodeA::NodeA(std::string name) : name_{std::move(name)} {}

NodeA::~NodeA() = default;

void NodeA::setPartner(NodeB* b) {
    partner_ = b;
}

std::string NodeA::describe() const {
    std::string result = "NodeA(\"" + name_ + "\")";
    if (partner_ != nullptr) {
        result += " -> partner: NodeB(\"" + partner_->name() + "\")";
    }
    return result;
}
