// 容器适配器：stack / queue / priority_queue。

#include <functional>
#include <queue>
#include <stack>
#include <vector>

#include <gtest/gtest.h>

TEST(Stack, LIFO) {
    std::stack<int> s;
    s.push(1);
    s.push(2);
    s.push(3);
    EXPECT_EQ(s.top(), 3);
    s.pop();
    EXPECT_EQ(s.top(), 2);
    EXPECT_EQ(s.size(), 2U);
}

TEST(Stack, CustomUnderlyingContainer) {
    // stack<int, vector<int>> 把 vector 作为底层容器：
    // vector 缓存效率好；不像默认 deque 在小数据时常更慢。
    std::stack<int, std::vector<int>> s;
    s.push(1);
    s.push(2);
    EXPECT_EQ(s.top(), 2);
}

TEST(Queue, FIFO) {
    std::queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    EXPECT_EQ(q.front(), 1);
    EXPECT_EQ(q.back(), 3);
    q.pop();
    EXPECT_EQ(q.front(), 2);
}

TEST(PriorityQueue, MaxHeapByDefault) {
    std::priority_queue<int> pq;
    for (int x : {3, 1, 4, 1, 5, 9, 2, 6}) {
        pq.push(x);
    }
    std::vector<int> out;
    while (!pq.empty()) {
        out.push_back(pq.top());
        pq.pop();
    }
    EXPECT_EQ(out, (std::vector{9, 6, 5, 4, 3, 2, 1, 1}));
}

TEST(PriorityQueue, MinHeapWithGreater) {
    std::priority_queue<int, std::vector<int>, std::greater<>> pq;
    for (int x : {3, 1, 4, 1, 5, 9, 2, 6}) {
        pq.push(x);
    }
    std::vector<int> out;
    while (!pq.empty()) {
        out.push_back(pq.top());
        pq.pop();
    }
    EXPECT_EQ(out, (std::vector{1, 1, 2, 3, 4, 5, 6, 9}));
}

TEST(PriorityQueue, ConstructFromRangeBuildsHeapInLinearTime) {
    // 通过迭代器对一次构造 → Floyd 建堆 O(n)
    std::vector<int> data{8, 3, 7, 2, 9, 1};
    std::priority_queue<int> pq{std::less<int>{}, data};
    EXPECT_EQ(pq.top(), 9);
    EXPECT_EQ(pq.size(), 6U);
}
