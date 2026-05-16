// 模块 12：stop_token 停止请求、查询与 stop_callback。

#include <atomic>
#include <stop_token>

#include <gtest/gtest.h>

TEST(StopToken, StopRequestedAfterRequest) {
    std::stop_source source;
    std::stop_token token = source.get_token();
    EXPECT_FALSE(token.stop_requested());
    source.request_stop();
    EXPECT_TRUE(token.stop_requested());
}

TEST(StopToken, CallbackInvokedOnRequestStop) {
    std::stop_source source;
    std::stop_token token = source.get_token();
    std::atomic<bool> invoked{false};
    std::stop_callback callback(token, [&invoked] { invoked.store(true); });
    EXPECT_FALSE(invoked.load());
    source.request_stop();
    EXPECT_TRUE(invoked.load());
}

TEST(StopToken, CallbackMayRunImmediatelyIfAlreadyStopped) {
    std::stop_source source;
    source.request_stop();
    std::stop_token token = source.get_token();
    bool ran = false;
    std::stop_callback callback(token, [&ran] { ran = true; });
    EXPECT_TRUE(ran);
}
