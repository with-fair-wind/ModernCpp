# clangd 排障手册

先保留日志和最终编译命令，再修改配置。每次只改变一个变量并重复同一验证用例。

## 标准顺序

1. 记录 `clangd --version` 和编辑器实际启动参数。
2. 运行 `clangd --check=<representative-source> --log=verbose`。
3. 查找配置解析警告、编译命令数据库路径、最终编译命令和命令是否从其他文件推断。
4. 将最终命令与成功构建该翻译单元的命令比较。
5. 修正编译命令数据库生成或编译器驱动程序发现，再处理 clang-tidy、clang-format 和性能。

## 症状与证据

| 症状 | 首先确认 | 常见修复 |
|---|---|---|
| 项目头文件找不到 | 编译命令数据库是否包含当前源文件及正确的 `-I`/`/I` | 重新配置（configure）；修正构建目标的头文件搜索路径；指向正确的编译命令数据库 |
| 标准库头文件找不到 | `argv[0]`、目标三元组、系统头文件搜索路径、query-driver 日志 | 使用真实编译器驱动程序的绝对路径；配置范围严格受限的 `--query-driver` |
| 编辑器红线但构建成功 | clangd 最终命令与构建命令差异 | 修复编译命令数据库，不在 `.clangd` 中盲目追加宏和头文件搜索路径 |
| 头文件使用错误命令 | 日志是否显示 `inferred from` 其他源文件 | 添加或打开最相关的翻译单元；必要时用工具补全头文件条目 |
| 新文件解析错误 | 它是否尚未进入构建图 | 重新配置；临时文件仅用最小后备参数，随后加入构建目标 |
| 生成头文件缺失 | 配置或构建步骤是否先生成该文件，路径是否存在 | 运行项目生成步骤并刷新编译命令数据库 |
| MinGW/交叉编译目标解析错误 | 驱动程序名称是否带目标前缀，query-driver 是否允许执行 | 改用带目标前缀的驱动程序或范围严格受限的白名单 |
| clang-cl 报输出文件参数冲突 | 最终命令是否含 `/Fo`、`/Fd`、`/FS` | 仅移除已确认无关的输出/PDB 参数 |
| clang-tidy 没有诊断 | `.clang-tidy`、`Add`/`Remove`、`FastCheckFilter`、有效检查列表 | 先查 `--help`；使用受支持的 `--verify-config`、`-list-checks`、`--explain-config` |
| clang-tidy 误报很多 | 是否检查了不必要的翻译单元或输出了第三方/系统头文件诊断，是否一次开启大组 | 分别限定输入与头文件诊断过滤条件；渐进启用；调整选项 |
| 格式化与 CI 不同 | 二者版本和读取的 `.clang-format` | 锁定兼容版本；确认最近的配置文件；执行只读检查 |
| CPU/内存过高 | 后台索引范围、并发、生成/第三方目录 | PathMatch 跳过非项目代码；限制 `-j`；评估外部索引 |

## 编译命令数据库检查

不要只确认文件存在。抽查 JSON 中：

- `file` 是否是预期翻译单元；
- `directory` 是否存在，且相对路径能从这里解析；
- `command` 或 `arguments` 中的编译器驱动程序是否存在；
- `-std`/`/std:`、`-D`/`/D`、`-I`/`/I`、`--target` 是否正确；
- 依赖管理器生成的头文件搜索路径是否属于当前构建配置；
- 同一文件是否存在互相冲突的多条命令。

CMake 的 `CMAKE_EXPORT_COMPILE_COMMANDS` 主要由 Makefile 和 Ninja 生成器实现；不要假设 Visual Studio/Xcode 生成器一定产生编译命令数据库。若使用 Ninja Multi-Config，仍应检查当前配置对应的命令是否满足预期。

## 驱动查询安全

clangd 只会执行编译命令 `argv[0]` 中的编译器驱动程序，且该路径必须与 `--query-driver` glob 匹配。日志应明确显示驱动程序已获允许，以及提取出的系统头文件搜索路径。若未匹配：

- 统一路径分隔符和大小写预期；
- 确认编译命令数据库中的 `argv[0]` 与 glob 匹配，而不是只匹配你在终端输入的别名；
- 避免把包装程序、脚本目录或用户可写的宽泛目录加入白名单；
- 不要为了“让它工作”使用单个 `*`。

## 配置作用域与合并

- 项目 `.clangd` 的路径条件相对配置片段所在目录匹配；用户级配置按绝对路径匹配。
- 多个匹配片段会组合，标量冲突由更高优先级片段决定；用户配置的优先级高于项目配置。
- `.clang-format` 使用离源文件最近的配置；子目录增量继承需 `BasedOnStyle: InheritParentConfig`。
- `.clang-tidy` 使用最近配置；子目录要叠加父级需 `InheritParentConfig: true`。
- clangd 会合并 `.clang-tidy` 与 `.clangd` 中的 clang-tidy 设置；冲突时 `.clangd` 优先，同一 clangd 配置中的 `Remove` 优先于 `Add`。
- CLI 参数和编辑器初始化选项可能覆盖项目发现路径；排障时必须记录实际启动参数。

## 何时清缓存或重启

先从日志证明 clangd 未重新加载编译命令数据库或配置，或索引确实损坏，再重启语言服务器。删除索引缓存是最后手段，因为它会触发完整重建，且无法修复错误的编译命令。

## 最小复现

若配置和命令正确但仍出现误诊断：

1. 保留产生问题的编译命令；
2. 缩减到最小源文件和必要头；
3. 用相同 clangd/Clang 前端版本复现；
4. 确认不是 clangd 为响应性跳过头文件函数体等已知限制；
5. 带版本、日志和最小复现向 LLVM issue tracker 报告。

## 上游依据

- clangd 故障排除：<https://clangd.llvm.org/troubleshooting>
- clangd FAQ：<https://clangd.llvm.org/faq>
- 编译命令：<https://clangd.llvm.org/design/compile-commands>
- clangd 索引设计：<https://clangd.llvm.org/design/indexing>
- Include Cleaner：<https://clangd.llvm.org/design/include-cleaner>
