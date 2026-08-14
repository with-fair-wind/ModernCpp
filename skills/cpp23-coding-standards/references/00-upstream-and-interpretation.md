# 00 · 规则依据与解释

## 使用顺序

1. 先遵循用户要求、项目文档、支持矩阵和工具配置。
2. 用 ISO C++23 标准文本及已采纳 WG21 提案判断语言和标准库语义。
3. 用 C++ Core Guidelines 等工程指南形成默认建议，不把指南建议误写成语言要求。
4. 用编译器和标准库官方文档判断实现支持；最终以项目构建矩阵和测试结果为准。

## 解释规则

- 违反后会使程序 ill-formed、产生未定义行为、破坏资源/并发安全或违反项目明确契约的规则，使用“必须/禁止”。
- 主要改善可读性、可维护性或降低误用概率的工程默认，使用“应/不应”，并保留有证据的例外。
- 尚未进入 C++23 或只在部分工具链实现的特性，不作为无条件要求；使用功能测试宏和支持矩阵判断。
- WG21 提案用于解释已采纳特性的设计和语义，不以提案编号代替项目兼容性验证。

## 上游资料

- C++23 工作草案 N4950：<https://wg21.link/N4950>
- WG21 SD-6 功能测试建议：<https://isocpp.org/std/standing-documents/sd-6-sg10-feature-test-recommendations>
- C++ Core Guidelines：<https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>
- `std::expected` P0323：<https://wg21.link/P0323>
- `std::print` P2093：<https://wg21.link/P2093>
- `std::mdspan` P0009：<https://wg21.link/P0009>
- `std::generator` P2502：<https://wg21.link/P2502>
- `[[assume]]` P1774：<https://wg21.link/P1774>
- `std::unreachable` P0627：<https://wg21.link/P0627>
