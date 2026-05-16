// 模块 13 测试：协程 Generator 的 co_yield / co_return 序列。

#include <vector>

#include <gtest/gtest.h>

#if defined(__cpp_impl_coroutine) && __cpp_impl_coroutine >= 201304L

#include <coroutine>
#include <exception>
#include <optional>
#include <utility>

namespace {

struct Sentinel {};

template <typename T>
class Generator {
public:
    struct promise_type {  // NOLINT(readability-identifier-naming)
        std::optional<T> current_value;

        Generator get_return_object() {  // NOLINT(readability-identifier-naming)
            return Generator{Handle::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept {  // NOLINT(readability-identifier-naming)
            return {};
        }
        std::suspend_always final_suspend() noexcept {  // NOLINT(readability-identifier-naming)
            return {};
        }

        // NOLINTNEXTLINE(readability-identifier-naming)
        std::suspend_always yield_value(T value) noexcept {
            current_value = std::move(value);
            return {};
        }

        void return_void() noexcept {}  // NOLINT(readability-identifier-naming)

        void unhandled_exception() {  // NOLINT(readability-identifier-naming)
            std::rethrow_exception(std::current_exception());
        }
    };

    using Handle = std::coroutine_handle<promise_type>;

    explicit Generator(Handle h) noexcept : handle_(h) {}

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    Generator(Generator&& other) noexcept : handle_(std::exchange(other.handle_, {})) {}

    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            destroy();
            handle_ = std::exchange(other.handle_, {});
        }
        return *this;
    }

    ~Generator() {
        destroy();
    }

    struct Iterator {
        Handle* owner_{nullptr};

        Iterator& operator++() {
            if (owner_ && *owner_ && !owner_->done()) {
                owner_->resume();
            }
            return *this;
        }

        friend bool operator!=(const Iterator& it, Sentinel /*unused*/) noexcept {
            if (!it.owner_) {
                return false;
            }
            const Handle h = *it.owner_;
            if (!h) {
                return false;
            }
            return !h.done();
        }

        T operator*() const {
            // 仅在 it != end() 时调用；此处协程已 yield，current_value 必有值。
            // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
            return *(*owner_).promise().current_value;
        }
    };

    Iterator begin() {
        if (!handle_ || handle_.done()) {
            return Iterator{nullptr};
        }
        handle_.resume();
        return Iterator{&handle_};
    }

    static Sentinel end() noexcept {
        return {};
    }

private:
    void destroy() noexcept {
        if (handle_) {
            handle_.destroy();
            handle_ = {};
        }
    }

    Handle handle_{};
};

Generator<int> makeSequence() {
    co_yield 2;
    co_yield 4;
    co_yield 6;
    co_return;
}

std::vector<int> collectAll(Generator<int> gen) {
    std::vector<int> out;
    for (auto it = gen.begin(); it != Generator<int>::end(); ++it) {
        out.push_back(*it);
    }
    return out;
}

TEST(Coroutine, YieldedValuesMatchOrder) {
    auto values = collectAll(makeSequence());
    ASSERT_EQ(values.size(), 3U);
    EXPECT_EQ(values[0], 2);
    EXPECT_EQ(values[1], 4);
    EXPECT_EQ(values[2], 6);
}

}  // namespace

#else

TEST(Coroutine, SkippedWhenUnavailable) {
    GTEST_SKIP() << "当前工具链未启用协程";
}

#endif
