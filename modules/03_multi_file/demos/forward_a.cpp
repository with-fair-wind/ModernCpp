// NodeA 的实现 —— 这里可以 #include "forward_b.h" 来使用 NodeB 的成员。
// 头文件里只有前置声明，因此只在需要完整类型时才引入完整定义。

#include "forward_a.h"

#include <string>
#include <utility>

#include "forward_b.h"  // 需要 NodeB 的完整定义以调用其成员函数

NodeA::NodeA(std::string name) : name_{std::move(name)} {}

NodeA::~NodeA() = default;

void NodeA::setPartner(NodeB* b) { partner_ = b; }

std::string NodeA::describe() const {
    std::string result = "NodeA(\"" + name_ + "\")";
    if (partner_ != nullptr) {
        // 此处需要 NodeB 的完整定义来调用 describe()
        result += " -> partner: " + partner_->describe();
    }
    return result;
}
