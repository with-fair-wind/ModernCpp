// 模块 12 演示：std::stop_source / std::stop_token / std::stop_callback 协作式取消。

#include <atomic>
#include <chrono>
#include <iostream>
#include <stop_token>
#include <thread>

namespace {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::atomic<int> callback_invocation_count{0};

} // namespace

int main() {
    std::stop_source src{};
    std::stop_token tok = src.get_token();

    std::cout << "初始 stop_requested: " << std::boolalpha << tok.stop_requested() << '\n';

    // stop_callback：在请求停止时（或注册时若已停止）由实现调用回调。
    int serial = 0;
    std::stop_callback cb1(tok, [&serial] {
        ++serial;
        std::cout << "stop_callback #1 invoked, serial=" << serial << '\n';
        callback_invocation_count.fetch_add(1);
    });

    {
        std::stop_callback cb2(tok, [] {
            std::cout << "stop_callback #2 invoked\n";
            callback_invocation_count.fetch_add(1);
        });

        std::cout << "注册两个回调后仍 stop_requested=" << tok.stop_requested() << '\n';

        // 请求停止：所有与该 token 关联的回调将被触发。
        src.request_stop();

        std::cout << "request_stop 之后 stop_requested=" << tok.stop_requested() << '\n';
    }
    // cb2 离开作用域销毁；cb1 仍在。

    std::cout << "回调触发次数（近似）: " << callback_invocation_count.load() << '\n';

    // 再次请求停止是幂等的。
    src.request_stop();

    // 对已停止的 token 注册回调：实现可能在构造函数里同步调用回调。
    bool ran_immediately = false;
    std::stop_callback cb_late(tok, [&ran_immediately] {
        ran_immediately = true;
        std::cout << "对已停止 token 注册的回调被执行\n";
    });
    std::cout << "cb_late 构造后 ran_immediately=" << std::boolalpha << ran_immediately << '\n';

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return 0;
}
