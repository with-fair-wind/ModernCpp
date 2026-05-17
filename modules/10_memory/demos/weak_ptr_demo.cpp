// std::weak_ptr：观测但不延长生命周期；用法典型场景是打断 shared_ptr 成环。

#include <iostream>
#include <memory>

namespace {

void demonstrateObserver() {
    struct Payload {
        int serial{};
    };

    auto shared_obj = std::make_shared<Payload>();
    shared_obj->serial = 81;

    std::weak_ptr<Payload> watcher = shared_obj;

    std::cout << "建立弱引用后的 expired? " << (watcher.expired() ? "是" : "否") << '\n';

    if (auto strong = watcher.lock(); strong != nullptr) {
        std::cout << "弱引用升格成功，serial=" << strong->serial << '\n';
    }

    shared_obj.reset();

    std::cout << "目标释放后 expired? " << (watcher.expired() ? "是" : "否") << '\n';
}

void demonstrateCycleBreak() {
    struct Child;

    struct Parent {
        std::shared_ptr<Child> child;
    };

    struct Child {
        // 如果把这里换成 shared_ptr<Parent>，很容易与 Parent::child 形成环导致泄漏；
        // weak_ptr 只观测父对象，不改变其引用计数，析构仍可层层回收。
        std::weak_ptr<Parent> parent;
    };

    auto parent_handle = std::make_shared<Parent>();
    auto child_handle = std::make_shared<Child>();
    parent_handle->child = child_handle;
    child_handle->parent = parent_handle;

    std::cout << "Parent shared use_count（无环）应为 1: " << parent_handle.use_count() << '\n';
    std::cout << "Child  shared use_count 应为 1: " << child_handle.use_count() << '\n';
}

}  // namespace

int main() {
    demonstrateObserver();
    demonstrateCycleBreak();
    return 0;
}
