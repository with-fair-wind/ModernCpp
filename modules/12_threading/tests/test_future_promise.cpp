// 模块 12：future / promise 的值传递与异常传递。

#include <future>
#include <stdexcept>
#include <thread>

#include <gtest/gtest.h>

TEST(FuturePromise, ValuePropagatesThroughFuture) {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    std::thread producer([&prom] { prom.set_value(12345); });
    EXPECT_EQ(fut.get(), 12345);
    producer.join();
}

TEST(FuturePromise, ExceptionPropagatesThroughFutureGet) {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    prom.set_exception(std::make_exception_ptr(std::runtime_error("planned failure")));
    EXPECT_THROW(static_cast<void>(fut.get()), std::runtime_error);
}
