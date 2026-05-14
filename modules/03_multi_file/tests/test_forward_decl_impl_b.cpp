// 前置声明测试的 NodeB 实现 TU。
// 内容与 demos/forward_b.cpp 相同，独立编译避免链接冲突。

#include <string>
#include <utility>

#include "forward_a.h"
#include "forward_b.h"

NodeB::NodeB(std::string name) : name_{std::move(name)} {}

NodeB::~NodeB() = default;

void NodeB::setPartner(NodeA* a) {
    partner_ = a;
}

std::string NodeB::describe() const {
    std::string result = "NodeB(\"" + name_ + "\")";
    if (partner_ != nullptr) {
        result += " -> partner: NodeA(\"" + partner_->name() + "\")";
    }
    return result;
}
