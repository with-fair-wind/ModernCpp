// 链接属性测试的第二个 TU。
// 提供外部链接函数、extern const 变量的定义，以及内部链接函数。

#include "linkage_api.h"

// 外部链接函数定义
int publicAdd(int a, int b) { return a + b; }
int publicMultiply(int a, int b) { return a * b; }

// extern const 定义
extern const int kSharedConstant = 42;

// 内部链接函数（static）
static int secretComputation(int x) { return (x * x) + x; }

// 匿名命名空间
namespace {
int hiddenHelper(int x) { return x * 100; }
}  // namespace

// 公开接口间接调用内部函数
int callInternalHelper() { return secretComputation(3); }

int callAnonNsHelper() { return hiddenHelper(5); }
