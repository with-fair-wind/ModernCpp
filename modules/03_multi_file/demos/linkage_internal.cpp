// 链接属性演示：本文件包含外部链接函数的定义，以及内部链接的实现细节。

#include "linkage_api.h"

// ========== 外部链接 ==========

int publicAdd(int a, int b) { return a + b; }

int publicMultiply(int a, int b) { return a * b; }

// extern const —— 显式外部链接的 const 变量
extern const int kSharedConstant = 42;

// ========== 内部链接（static）==========
// static 函数只在本 TU 可见，即使其他 TU 声明了同名原型也链接不到

static int internalHelper(int x) { return (x * x) + 1; }

// ========== 内部链接（匿名命名空间）==========

namespace {
int anonHelper(int x) { return (x * 3) - 2; }
}  // namespace

// 通过公开接口间接暴露内部函数的计算结果
int callInternalHelper() { return internalHelper(5); }

int callAnonNsHelper() { return anonHelper(7); }
