#ifndef MCPP_03_MULTI_FILE_LINKAGE_API_H
#define MCPP_03_MULTI_FILE_LINKAGE_API_H

// 链接属性演示的公共接口。
// 外部链接的函数/变量在此声明，内部链接的实现细节隐藏在源文件中。

// 外部链接函数 —— 默认行为，可跨 TU 访问
int publicAdd(int a, int b);
int publicMultiply(int a, int b);

// extern const —— 显式赋予 const 变量外部链接
extern const int kSharedConstant;

// 通过间接接口暴露内部实现的结果（用于验证内部链接函数确实存在）
int callInternalHelper();
int callAnonNsHelper();

#endif  // MCPP_03_MULTI_FILE_LINKAGE_API_H
