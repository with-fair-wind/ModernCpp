// NodeB 的实现 —— 同样在此处引入 NodeA 的完整定义。

#include "forward_b.h"

#include <string>
#include <utility>

#include "forward_a.h"  // 需要 NodeA 的完整定义以调用其成员函数

NodeB::NodeB(std::string name) : name_{std::move(name)} {}

NodeB::~NodeB() = default;

void NodeB::setPartner(NodeA* a) { partner_ = a; }

std::string NodeB::describe() const {
    std::string result = "NodeB(\"" + name_ + "\")";
    if (partner_ != nullptr) {
        result += " -> partner: NodeA(\"" + partner_->describe() + "\")";
    }
    return result;
}
