# 编辑器与用户级配置

先确认编辑器实际启动的是哪个 `clangd`，再配置客户端。团队共享的解析与诊断策略放在仓库 `.clangd`；编译器绝对路径、`--query-driver`、日志级别和并发数等机器相关参数放在用户或编辑器配置中。

## 通用检查

1. 记录实际 `clangd` 可执行文件、版本和完整启动参数。
2. 确认只启用一个负责 C/C++ 诊断与补全的语言服务器，避免与其他 IntelliSense 后端重复报告。
3. 确认编辑器打开的工作区根目录能发现预期 `.clangd` 与编译命令数据库。
4. 在 clangd 日志中核对配置文件、编译命令数据库路径、最终编译命令和 `argv[0]`。
5. 若要使用 clangd 格式化，明确把 clangd 扩展设为 C/C++ 的格式化提供程序。
6. 使用容器、WSL 或远程主机时，确认 clangd 与实际构建位于同一文件系统命名空间；编译命令中的工作目录、驱动程序、sysroot 和生成文件必须在 clangd 所在环境可见。

用户级 `config.yaml` 的常见位置：

- Windows：`%LocalAppData%\clangd\config.yaml`
- macOS：`~/Library/Preferences/clangd/config.yaml`
- Linux：`$XDG_CONFIG_HOME/clangd/config.yaml`，通常为 `~/.config/clangd/config.yaml`

用户级配置的优先级高于项目配置；项目内层 `.clangd` 的优先级高于外层 `.clangd`。用户级配置中的 `If.PathMatch` 按文件绝对路径匹配，项目配置中则按相对配置文件目录的路径匹配。

## Visual Studio Code

使用官方 clangd 扩展 `llvm-vs-code-extensions.vscode-clangd`。以下用户设置仅作结构示例，路径必须替换为本机可信工具链目录，不要原样提交到仓库：

```jsonc
{
  "clangd.path": "D:/tools/llvm/bin/clangd.exe",
  "clangd.arguments": [
    "--query-driver=D:/toolchains/ucrt64/bin/*-g++.exe,D:/toolchains/ucrt64/bin/*-gcc.exe"
  ],
  "[cpp]": {
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
  },
  "[c]": {
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd"
  }
}
```

禁用其他扩展中重叠的 C/C++ IntelliSense 功能，或明确只保留其调试等非语言服务器能力。修改后检查 clangd 输出面板，确认实际命令行与预期一致。

## Neovim 与其他 LSP 客户端

在客户端的 clangd `cmd` 列表中追加机器相关参数；具体注册 API 随 Neovim 与 LSP 插件版本变化，应以当前客户端文档为准：

```lua
cmd = {
  "clangd",
  "--query-driver=/opt/toolchains/aarch64/bin/aarch64-linux-gnu-*",
}
```

CLion 或其他 IDE 可能使用内置、捆绑或自定义 clangd。先在 IDE 设置和日志中确认所选后端、二进制版本与启动参数；不要假设编辑器终端中的 `clangd` 就是 IDE 实际使用的程序。

## 验证与排障

- 比较编辑器日志中的 `clangd --version` 与命令行版本。
- 确认 `--query-driver` glob 匹配编译命令 `argv[0]` 的真实路径。
- 确认没有同时存在项目级 `--compile-commands-dir` 与用户级覆盖，导致读取不同的编译命令数据库。
- 修改 `.clangd` 后先观察日志是否自动重新加载；只有未生效时才重启语言服务器。
- 格式化结果不一致时，分别记录编辑器使用的格式化提供程序、`clang-format` 版本和读取到的 `.clang-format`。

## 上游依据

- clangd 编辑器集成：<https://clangd.llvm.org/installation#editor-plugins>
- clangd 配置：<https://clangd.llvm.org/config>
- Visual Studio Code clangd 扩展：<https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd>
