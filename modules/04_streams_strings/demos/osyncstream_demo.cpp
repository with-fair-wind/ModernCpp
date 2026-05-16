// std::osyncstream：多线程并行写共享 std::cout 时由 syncbuf 串行移交；演示 emit/noemit flush 语义。

#if defined(__cpp_lib_syncbuf) && (__cpp_lib_syncbuf >= 201803L)
#include <chrono>
#include <iostream>
#include <sstream>
#include <syncstream>
#include <thread>
#include <vector>

namespace {

constexpr int kWriterThreads = 4;
constexpr auto kBurstDelay = std::chrono::milliseconds{30};

void compareEmitPolicies() {
    std::ostringstream recorder{};
    {
        std::osyncstream watcher{recorder};
        watcher << std::emit_on_flush;
        watcher << "tick";
        watcher.flush();
        watcher << "tock";
        watcher.flush();
    }
    {
        std::osyncstream muted{recorder};
        muted << std::noemit_on_flush << "silent";
        muted.flush();
        muted << "-tail";
        // 析构时提交整段缓冲区
    }
    std::cout << "ostringstream 快照（emit/noemit）：\"" << recorder.str() << "\"\n";
}

void emitWorkerBurst(int marker) {
    for (int round = {}; round < kWriterThreads / 2; ++round) {
        std::osyncstream guarded{std::cout};
        guarded << "worker#" << marker << " round=" << round << '\n';
    }
    std::this_thread::sleep_for(kBurstDelay);
}

void spawnParallelWriters() {
    std::cout << "--- 并行写共享 std::cout（由 osyncstream 串行化） ---\n";
    std::vector<std::thread> workers{};
    workers.reserve(static_cast<std::size_t>(kWriterThreads));
    for (int index = {}; index < kWriterThreads; ++index) {
        workers.emplace_back(emitWorkerBurst, index);
    }
    for (auto& worker : workers) {
        worker.join();
    }
}

}  // namespace

#else

#include <iostream>

namespace {

void printFallbackNotice() {
    std::cout << "当前 STL 未提供 <syncstream> 或未定义 __cpp_lib_syncbuf——跳过。\n";
}

}  // namespace

#endif

int main() {
#if defined(__cpp_lib_syncbuf) && (__cpp_lib_syncbuf >= 201803L)
    compareEmitPolicies();
    spawnParallelWriters();
#else
    printFallbackNotice();
#endif
    return 0;
}
