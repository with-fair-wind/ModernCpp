# CI 使用与配置完整指南

> 本文档详细介绍本项目使用的 GitHub Actions CI 工作流：是什么、怎么用、每段配置的含义、如何观察、如何调试，以及 CI 进阶玩法。
>
> 配套文件：[`.github/workflows/ci.yml`](../.github/workflows/ci.yml)

---

## 目录

1. [CI 是什么](#1-ci-是什么)
2. [GitHub Actions 基础概念](#2-github-actions-基础概念)
3. [本项目 ci.yml 逐段解析](#3-本项目-ciyml-逐段解析)
4. [触发时机](#4-触发时机)
5. [Matrix 并行策略](#5-matrix-并行策略)
6. [缓存与加速](#6-缓存与加速)
7. [日常使用流程](#7-日常使用流程)
8. [观察 CI 运行结果](#8-观察-ci-运行结果)
9. [调试 CI 失败](#9-调试-ci-失败)
10. [本地复现 CI](#10-本地复现-ci)
11. [扩展功能](#11-扩展功能)
12. [分支保护与必需检查](#12-分支保护与必需检查)
13. [Secrets 与权限](#13-secrets-与权限)
14. [费用与配额](#14-费用与配额)
15. [常见问题 FAQ](#15-常见问题-faq)

---

## 1. CI 是什么

**CI（Continuous Integration，持续集成）** 是一种工程实践：**每次代码变更都自动触发构建与测试**，让"在我电脑上能跑"的问题不再传染给团队。

### 没有 CI 的世界

```
开发者 A 提交代码 → 本地能编译 → push
                                     │
                                     ▼
开发者 B pull → 编译失败 → 浪费 30 分钟排查 → 发现 A 漏提交了文件
```

### 有 CI 的世界

```
开发者 A 提交 PR → 云端自动跑 build + test → 红叉 → A 立刻修复 → 绿勾才能合并
                                                            │
                                                            ▼
                                          开发者 B 永远拉到能编译的代码
```

### CI 在本项目的价值

本项目目标是**支持 GCC / Clang / MSVC 三种编译器**。开发者只可能在一种环境下写代码，剩下两种全靠 CI 验证：

- 本地用 GCC 写的代码，可能 MSVC 编不过（C++23 库支持差异）
- 本地用 Linux 写的代码，可能 Windows 编不过（路径分隔符、大小写）
- 修了一处可能引入静态检查/格式问题

**CI 是这种多环境项目的"保险丝"**。

---

## 2. GitHub Actions 基础概念

### 2.1 它是什么

GitHub 自家的 CI/CD 平台。开公开仓库**完全免费**，私有仓库**每月 2000 分钟免费**。

### 2.2 核心术语

| 术语 | 含义 | 本项目对应 |
|---|---|---|
| **Workflow** | 一个 .yml 文件 = 一个工作流 | `.github/workflows/ci.yml` |
| **Job** | Workflow 里的独立任务，跑在自己的虚拟机 | `build-test`、`lint` |
| **Step** | Job 里的一步命令（脚本或 action 调用） | "Configure"、"Build" 等 |
| **Runner** | 跑 job 的虚拟机 | `ubuntu-24.04` / `windows-2022` |
| **Action** | 别人写好的可复用步骤 | `actions/checkout@v4` 等 |
| **Matrix** | 让一个 job 用不同参数并行跑 N 次 | linux-gcc / linux-clang / windows-msvc |
| **Artifact** | 跑完后保留的文件（二进制、报告） | 本项目暂未使用 |
| **Secret** | 加密存储的密钥（API token 等） | 本项目暂未使用 |

### 2.3 文件位置约定

```
项目根/
└── .github/
    └── workflows/
        ├── ci.yml          ← 必须在这个目录下，文件名任意
        ├── release.yml     ← 一个仓库可以有多个 workflow
        └── nightly.yml
```

GitHub 自动扫描 `.github/workflows/*.yml`，每个文件独立的 workflow。

### 2.4 整体执行流程

```
开发者 push / 开 PR
        │
        ▼
GitHub 检测到 .github/workflows/*.yml 中的触发条件匹配
        │
        ▼
为每个 job 启动一台干净的虚拟机
   - actions/checkout 把代码 clone 进去
   - 一步步执行 steps
        │
        ▼
所有 step 退出码为 0 → ✅ Job 成功
任意 step 退出码非 0 → ❌ Job 失败
        │
        ▼
结果回写到 GitHub
   - PR 页面显示 ✓/✗
   - 状态徽章更新
   - 邮件通知
```

---

## 3. 本项目 ci.yml 逐段解析

完整文件见 [`.github/workflows/ci.yml`](../.github/workflows/ci.yml)，以下逐段拆解。

### 3.1 顶部元信息

```yaml
name: CI
```
- Workflow 显示名，出现在 Actions 页面顶端、PR 状态栏、徽章里

### 3.2 触发条件

```yaml
on:
  push:
    branches: [master]
  pull_request:
    branches: [master]
  workflow_dispatch:
```
- `push` 到 master：合并后再验证主线没坏
- `pull_request` 目标 master：PR 阶段就拦截
- `workflow_dispatch`：在 GitHub Actions 页面手动按按钮也能触发，方便临时跑

### 3.3 并发控制

```yaml
concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true
```
**用途**：同一分支连续两次 push，前一次 CI 还没跑完就被取消，只跑最新的。
- 省时省钱
- 避免老 commit 的绿勾误导

### 3.4 build-test job 头

```yaml
jobs:
  build-test:
    name: ${{ matrix.name }}
    runs-on: ${{ matrix.os }}
```
- `name` 用 matrix 变量动态生成（PR 页面显示 `linux-gcc`、`windows-msvc` 而不是统一的 `build-test`）
- `runs-on` 指定虚拟机镜像（也由 matrix 决定）

### 3.5 Matrix 矩阵

```yaml
strategy:
  fail-fast: false
  matrix:
    include:
      - name: linux-gcc
        os: ubuntu-24.04
        preset: gcc-relwithdebinfo
        cc: gcc-13
        cxx: g++-13
        triplet: x64-linux
      - name: linux-clang
        ...
      - name: windows-msvc
        ...
```
- `fail-fast: false`：一个 job 挂了，不取消其它，方便一次拿到全部失败信息
- `include`：显式列举 3 组参数（比组合写法 `os: [...] compiler: [...]` 更直观）

### 3.6 vcpkg 二进制缓存环境

```yaml
env:
  VCPKG_BINARY_SOURCES: "clear;x-gha,readwrite"
```
告诉 vcpkg：**把已编译好的库（如 gtest）缓存到 GitHub Actions 缓存系统中**。第二次跑直接拉缓存，从 5 分钟变 30 秒。

### 3.7 steps 详解

#### Step 1：暴露缓存 token

```yaml
- name: Export GitHub Actions cache env (vcpkg binary cache)
  uses: actions/github-script@v7
  with:
    script: |
      core.exportVariable('ACTIONS_CACHE_URL', process.env.ACTIONS_CACHE_URL || '');
      core.exportVariable('ACTIONS_RUNTIME_TOKEN', process.env.ACTIONS_RUNTIME_TOKEN || '');
```
把 GitHub 内部的两个 token 导出为环境变量，vcpkg 拿这两个值才能读写缓存。属于 boilerplate。

#### Step 2：clone 代码

```yaml
- uses: actions/checkout@v4
```
- 把仓库 clone 到 `${{ github.workspace }}`（默认 `/home/runner/work/<repo>/<repo>`）
- `@v4` 钉一个大版本，自动拿 patch 修复

#### Step 3：装编译器

```yaml
- name: Install GCC 13 (Linux)
  if: matrix.name == 'linux-gcc'
  run: sudo apt-get update && sudo apt-get install -y gcc-13 g++-13 ninja-build
```
- `if:` 条件执行，只在 linux-gcc job 跑
- `run:` 执行 shell 命令（Linux 默认 bash，Windows 默认 PowerShell）

clang job 装 clang-18 + libstdc++-13（C++23 库要新版 libstdc++）。

#### Step 4：MSVC 环境

```yaml
- name: Setup MSVC dev environment
  if: runner.os == 'Windows'
  uses: ilammy/msvc-dev-cmd@v1
  with:
    arch: x64
```
**第三方 action**，相当于自动跑 `vcvars64.bat`。后续 step 的 PowerShell 里 `cl.exe`、`link.exe` 直接可用。

#### Step 5：装 vcpkg

```yaml
- name: Setup vcpkg
  uses: lukka/run-vcpkg@v11
  with:
    vcpkgGitCommitId: '5ee5eee0d3e9c6098b24d263e9099edcdcef6631'
```
- 装 vcpkg 到 `${{ github.workspace }}/vcpkg`
- **钉 commit ID 保证可重现**——不要写 `master`，否则 vcpkg 升级会让你的 CI 历史前后不一致

#### Step 6：configure / build / test

```yaml
- name: Configure (Linux)
  if: runner.os == 'Linux'
  env:
    CC:  ${{ matrix.cc }}
    CXX: ${{ matrix.cxx }}
    VCPKG_ROOT: ${{ github.workspace }}/vcpkg
  run: cmake --preset ${{ matrix.preset }}

- name: Build (Linux)
  if: runner.os == 'Linux'
  run: cmake --build --preset ${{ matrix.preset }} --parallel

- name: Test (Linux)
  if: runner.os == 'Linux'
  run: ctest --preset ${{ matrix.preset }}
```
- `env:` 给单步注入环境变量（指定编译器、指定 vcpkg 根）
- `--parallel` 让 ninja 用全部 CPU 核心
- ctest 退出码非零 = 测试失败 = job 红叉

Windows 段同理，只是用 `shell: pwsh` 显式 PowerShell。

### 3.8 lint job

```yaml
lint:
  name: clang-format check
  runs-on: ubuntu-24.04
  steps:
    - uses: actions/checkout@v4
    - name: Install clang-format 18
      run: sudo apt-get update && sudo apt-get install -y clang-format-18
    - name: Verify formatting
      run: |
        mapfile -t files < <(git ls-files '*.cpp' '*.hpp' '*.h' '*.cc' '*.cxx')
        if [ ${#files[@]} -eq 0 ]; then
          echo "No C++ sources to check."
          exit 0
        fi
        clang-format-18 --dry-run --Werror "${files[@]}"
```
**与 build-test 并行运行**的独立 job：
- `--dry-run` 不修改文件
- `--Werror` 发现需要修改即报错退出非零
- `git ls-files` 只检查 git 跟踪的文件，避免扫到 build 产物

---

## 4. 触发时机

完整触发器列表（你可以混合多种）：

| 触发器 | 用途 |
|---|---|
| `push` | 推到指定分支时跑 |
| `pull_request` | PR 创建/更新时跑 |
| `pull_request_target` | PR 跑，但用目标分支的 yml（更安全，但有限制） |
| `schedule` | 定时跑（cron 表达式） |
| `workflow_dispatch` | 手动按钮触发 |
| `workflow_run` | 另一个 workflow 完成后触发 |
| `release` | 创建 release 时 |
| `tag` | 推 tag 时 |
| `issue` / `issue_comment` | issue 互动 |
| `repository_dispatch` | API 远程触发 |

**本项目用的组合**（`push + pull_request + workflow_dispatch`）是最经典的"保险栓"组合：

- PR 阶段就拦截 → 防止坏代码合入
- master 上 push 后再跑 → 防止合并冲突解决错误等遗漏
- 手动触发 → 临时调试不用打无意义 commit

---

## 5. Matrix 并行策略

### 5.1 为什么用 Matrix

本项目要验证 3 个编译器。如果不用 matrix，要写 3 段几乎一样的 job 配置，重复且难维护。

Matrix 让一段 job 自动复制 N 份并行跑，每份用不同变量。

### 5.2 两种写法

**Include 写法（本项目用的）**：每组参数显式列出。

```yaml
matrix:
  include:
    - name: linux-gcc
      os: ubuntu-24.04
      preset: gcc-relwithdebinfo
    - name: windows-msvc
      os: windows-2022
      preset: msvc
```
- 适合**参数差异较大、组合稀疏**的场景

**笛卡尔积写法**：所有数组的笛卡尔积。

```yaml
matrix:
  os: [ubuntu-24.04, windows-2022]
  build_type: [Debug, Release, RelWithDebInfo]
  # 自动生成 2 × 3 = 6 个 job
```
- 适合**所有组合都合法**的场景
- 可以用 `exclude:` 排除某些不合法组合

### 5.3 fail-fast 行为

```yaml
strategy:
  fail-fast: false   # 一个 job 挂了，不取消其它（推荐用于多平台）
  fail-fast: true    # 一个挂了立刻取消所有（默认值）
```

CI 调试期建议用 `false`，能一次拿到所有平台的失败信息。

### 5.4 Matrix 限制

- 单个 job 矩阵展开**不能超过 256 个**
- 每个 workflow 同时最多并行 **20 个 job**（免费账户）
- `runs-on` 也可以用 matrix 变量

---

## 6. 缓存与加速

### 6.1 vcpkg 二进制缓存（本项目使用）

vcpkg 装完 gtest 后会把编译产物上传到 GitHub Actions 缓存。下次跑直接下载。

```yaml
env:
  VCPKG_BINARY_SOURCES: "clear;x-gha,readwrite"
```

- `clear` 清掉默认缓存源
- `x-gha,readwrite` 启用 GitHub Actions 缓存，可读可写

### 6.2 通用 actions/cache

```yaml
- uses: actions/cache@v4
  with:
    path: |
      ~/.ccache
      build/_deps
    key: ${{ runner.os }}-build-${{ hashFiles('**/CMakeLists.txt') }}
    restore-keys: |
      ${{ runner.os }}-build-
```
- `key` 完全匹配才命中
- `restore-keys` 部分匹配也能拿到旧缓存
- `hashFiles` 让缓存随关键文件变化自动失效

### 6.3 ccache（可选加速）

```yaml
- uses: hendrikmuhs/ccache-action@v1.2
- run: |
    cmake --preset gcc-debug \
      -DCMAKE_C_COMPILER_LAUNCHER=ccache \
      -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```
- ccache 缓存编译产物，第二次构建可加速 80%+
- 本项目 demo 较小，暂不需要

### 6.4 actions/checkout 加速

```yaml
- uses: actions/checkout@v4
  with:
    fetch-depth: 1   # 只拉最新一次 commit（默认就是 1）
```

---

## 7. 日常使用流程

### 7.1 你不需要做任何事

CI 的设计哲学：**对开发者完全透明**。

```
你的工作流：
1. 在分支上写代码
2. git push
3. （CI 自动跑，你做别的事）
4. 收到通知 / 看 PR 页面，发现绿勾 → 找人 review → merge
   收到通知 / 看 PR 页面，发现红叉 → 点 Details 看日志 → 修复 → push
```

### 7.2 PR 流程的标准位置

```
1. git checkout -b feat/xxx
2. 写代码
3. git commit -m "feat: ..."
4. git push -u origin feat/xxx
5. 在 GitHub 网页开 PR
6. ⏳ 等 CI 跑完（PR 页面会有 progress 条）
7. 全绿 → 选 Squash and merge
   有红 → 点 Details → 看哪个 step 挂了 → 本地修 → 再次 push（PR 自动重跑 CI）
```

### 7.3 手动触发

某些场景想不带 commit 也跑一次（如怀疑某个间歇性问题）：

1. GitHub 网页 → Actions 标签
2. 左侧选 `CI` workflow
3. 右上角 `Run workflow` 按钮 → 选分支 → 跑

---

## 8. 观察 CI 运行结果

### 8.1 PR 页面

PR 页面底部一栏：

```
Some checks haven't completed yet
  🟡 CI / linux-gcc       Running...        Details
  🟡 CI / linux-clang     Running...        Details
  🟡 CI / windows-msvc    Running...        Details
  ✅ CI / clang-format    Successful in 30s  Details
```

跑完会变成：

```
All checks have passed
  ✅ CI / linux-gcc       Successful in 4m 23s   Details
  ✅ CI / linux-clang     Successful in 5m 1s    Details
  ✅ CI / windows-msvc    Successful in 8m 45s   Details
  ✅ CI / clang-format    Successful in 30s      Details
```

或者：

```
Some checks were not successful
  ❌ CI / linux-gcc       Failing after 2m 10s   Details
  ✅ CI / linux-clang     Successful in 5m 1s    Details
  ✅ CI / windows-msvc    Successful in 8m 45s   Details
  ✅ CI / clang-format    Successful in 30s      Details
```

点 **Details** → 跳到 job 详情页。

### 8.2 Actions 总览页

URL：`https://github.com/<owner>/<repo>/actions`

每行 = 一次 workflow run，显示：
- 触发原因（push / PR / 手动）
- 触发分支
- 触发者
- 总耗时
- 是否成功

### 8.3 Job 详情页

```
左侧：Steps 列表
   ✓ Set up job
   ✓ Export GitHub Actions cache env
   ✓ Run actions/checkout@v4
   ✓ Install GCC 13
   ✓ Setup vcpkg
   ✓ Configure (Linux)
   ✓ Build (Linux)
   ✗ Test (Linux)        ← 红叉，点开看日志
   ✓ Post Run actions/checkout@v4
   ✓ Complete job

右侧：选中 step 的完整日志
   [pass] BasicTest.PowerOfTwoHelpers
   [fail] BasicTest.PopcountAndLeadingZeros
        Expected: 4
        Got: 3
   ...
```

GitHub 会自动展开**最后一个失败的 step**，节省排查时间。

### 8.4 状态徽章

在 README 顶部加：

```markdown
[![CI](https://github.com/with-fair-wind/ModernCpp/actions/workflows/ci.yml/badge.svg)](https://github.com/with-fair-wind/ModernCpp/actions/workflows/ci.yml)
```

效果：[![CI passing]] 一眼看主线状态。

---

## 9. 调试 CI 失败

### 9.1 排查顺序

1. **看哪个 job 失败**：linux 还是 windows？
2. **看哪个 step 失败**：Configure 还是 Build 还是 Test？
3. **看日志末尾**：90% 答案在最后 30 行
4. **复现到本地**：能本地重现就用熟悉的工具调
5. **不能本地重现**：往 yml 里临时加 debug step

### 9.2 常见失败模式

| 失败位置 | 常见原因 |
|---|---|
| **Setup vcpkg** | vcpkg commit ID 失效 / 网络问题 |
| **Configure** | preset 名拼错、CMake 版本太老、编译器没装上 |
| **Build** | 跨编译器代码不兼容（最常见的是 MSVC 不支持的 C++23 特性、GCC 的 `[[no_unique_address]]`、Clang 的 ODR 警告） |
| **Test** | 测试本身有 bug、平台行为差异（浮点、locale、文件路径） |
| **clang-format** | 本地保存时未触发格式化 |

### 9.3 临时加日志

```yaml
- name: Debug environment
  run: |
    echo "PATH = $PATH"
    cmake --version
    g++-13 --version
    env | sort
```

push 跑完看完信息再删掉。

### 9.4 用 tmate 进 CI 虚拟机

终极武器，能 SSH 进 runner 调试：

```yaml
- name: Setup tmate session
  if: failure()
  uses: mxschmitt/action-tmate@v3
  timeout-minutes: 30
```
- `if: failure()` 仅失败时启动
- 跑起来后日志里会打印一个 SSH 命令，你 SSH 进去手动操作
- 用完关掉（or 等 timeout）

### 9.5 重跑

不改代码也能再跑一次（怀疑间歇问题）：

- Actions 页面 → 失败的 run → 右上 `Re-run failed jobs`（只重跑红的）
- 或 `Re-run all jobs`（全部重跑）

---

## 10. 本地复现 CI

### 10.1 act 工具

[act](https://github.com/nektos/act) 把 GitHub Actions 跑在本地 Docker 里。

```bash
# 安装
choco install act-cli   # Windows
brew install act        # macOS

# 跑全部 job
act

# 只跑某个 job
act -j build-test

# 只跑 push 事件
act push
```

**限制**：
- Windows job 跑不了（act 只支持 Linux 容器）
- 部分 GitHub-hosted 专用功能不可用
- 需要本地 Docker

### 10.2 直接跑 ci.yml 里的命令

最简单：把 yml 里的 `run:` 内容粘到本地 shell 跑。

```bash
# 模拟 linux-gcc job 的核心步骤
export VCPKG_ROOT=/path/to/vcpkg
export CC=gcc-13 CXX=g++-13
cmake --preset gcc-relwithdebinfo
cmake --build --preset gcc-relwithdebinfo --parallel
ctest --preset gcc-relwithdebinfo
```

---

## 11. 扩展功能

### 11.1 上传产物

```yaml
- uses: actions/upload-artifact@v4
  with:
    name: linux-gcc-binaries
    path: build/gcc-relwithdebinfo/bin/
    retention-days: 30
```

跑完后在 Actions 页面能下载，保留 30 天。

### 11.2 上传测试报告

```yaml
- name: Test (Linux)
  run: ctest --preset gcc-relwithdebinfo --output-junit test-results.xml

- uses: actions/upload-artifact@v4
  if: always()        # 失败时也上传
  with:
    name: test-results-${{ matrix.name }}
    path: test-results.xml
```

### 11.3 代码覆盖率

```yaml
- run: |
    cmake --preset gcc-debug -DCMAKE_CXX_FLAGS="--coverage"
    cmake --build --preset gcc-debug
    ctest --preset gcc-debug
    gcovr --xml -o coverage.xml

- uses: codecov/codecov-action@v4
  with:
    files: coverage.xml
```

### 11.4 自动 release

```yaml
on:
  push:
    tags: ['v*']

jobs:
  release:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - run: cmake --preset gcc-release && cmake --build --preset gcc-release
      - uses: softprops/action-gh-release@v2
        with:
          files: build/gcc-release/bin/*
```

打 tag → 自动 build → 上传到 GitHub Releases。

### 11.5 自动 PR 评论

```yaml
- uses: actions/github-script@v7
  with:
    script: |
      github.rest.issues.createComment({
        issue_number: context.issue.number,
        owner: context.repo.owner,
        repo: context.repo.repo,
        body: '✅ All checks passed!'
      })
```

### 11.6 定时任务

```yaml
on:
  schedule:
    - cron: '0 2 * * *'     # 每天凌晨 2 点 UTC
```

跑 nightly build、依赖更新检查、安全扫描。

---

## 12. 分支保护与必需检查

### 12.1 为什么要设置

CI 跑出红叉但开发者强行点 merge？**没有强制检查就会发生**。分支保护让红叉 = 网页上 merge 按钮变灰，连 admin 都默认不能合。

### 12.2 配置步骤

1. 仓库 `Settings → Branches → Branch protection rules → Add rule`
2. **Branch name pattern**：`master`
3. 勾选：
   - ✅ **Require a pull request before merging**
     - Required approvals: `1`（需要 N 人 approve）
   - ✅ **Require status checks to pass before merging**
     - Require branches to be up to date before merging（可选）
     - 在搜索框选中：
       - `CI / linux-gcc`
       - `CI / linux-clang`
       - `CI / windows-msvc`
       - `CI / clang-format check`
   - ✅ **Require conversation resolution before merging**（PR 评论必须解决）
   - ✅ **Do not allow bypassing the above settings**（连 admin 也不能绕过）

### 12.3 必需检查的副作用

- 红叉 PR 无法合并（按钮灰）
- master 分支无法直接 push（必须走 PR）
- 强制开发流程一致性

**强烈推荐开启**——这是 CI 真正发挥作用的关键开关。

---

## 13. Secrets 与权限

### 13.1 用 secret 注入密钥

仓库 `Settings → Secrets and variables → Actions → New repository secret`：

```
Name:  CRATES_API_TOKEN
Value: <你的 token>
```

yml 里引用：

```yaml
- run: cargo publish --token ${{ secrets.CRATES_API_TOKEN }}
```

GitHub 自动在日志里把 secret 值打码成 `***`。

### 13.2 内置 secret

```yaml
${{ secrets.GITHUB_TOKEN }}    # 自动生成的 token，可访问当前仓库的 API
```

无需手动配置，跑 `gh` CLI、调用 GitHub API 都用它。

### 13.3 权限收紧

```yaml
permissions:
  contents: read
  pull-requests: write    # 只在需要时给写权限
```

默认 token 权限较宽，**正式项目建议显式收紧**。

---

## 14. 费用与配额

### 14.1 GitHub-hosted runner 计费

| 镜像 | 计费倍率 | 含义 |
|---|---|---|
| Linux | 1× | 1 分钟扣 1 分钟 |
| Windows | 2× | 1 分钟扣 2 分钟 |
| macOS | 10× | 1 分钟扣 10 分钟 |

### 14.2 免费额度

| 账户类型 | 免费分钟/月 | 免费存储 |
|---|---|---|
| Personal 公开仓库 | **无限** | **无限** |
| Personal 私有仓库 | 2000 | 500 MB |
| Pro 私有 | 3000 | 1 GB |
| Team 私有 | 3000 | 2 GB |

**本项目是公开仓库 → 完全免费**。

### 14.3 自托管 runner

如果配额不够，可以自己提供机器：

- 任意一台 Linux/Windows/macOS 机器
- 装 GitHub Actions Runner agent
- 跑在公司机房 / 自己 PC / 树莓派
- 完全免费，配置 `runs-on: self-hosted`

---

## 15. 常见问题 FAQ

### Q1: CI 第一次运行好慢

正常。**首次没有缓存**：vcpkg 要从源码编译 gtest，10 分钟很正常。第二次走缓存只要 30 秒。

### Q2: 我电脑能编译，CI 不行

99% 的原因：

- 你装了某个 vcpkg 全局库 / 系统包，CI 没有
- 你 CMake/编译器版本和 CI 不一致（CI 用的是 ubuntu-24.04 自带版本）
- 路径分隔符问题（`\` vs `/`）
- 文件名大小写（你 Windows 不区分，Linux 区分）
- 隐藏的 `.gitignore` 文件没提交，CI 拿不到

### Q3: 改了 yml 不生效

- 必须 push 到 GitHub 才生效，本地 commit 不行
- yml 在哪个分支被改，PR 跑的就是 PR 分支的版本
- yml 语法错误的话 Actions 页面会显示 ⚠️ 警告

### Q4: 看不懂日志

复制最后 30-50 行 → 贴给 Claude/ChatGPT → 99% 一发命中。

### Q5: 想暂时跳过 CI

commit message 里加 `[skip ci]`：

```bash
git commit -m "docs: typo fix [skip ci]"
```

GitHub 会跳过该 commit 的所有 workflow。

### Q6: 缓存如何清除

Actions 页面 → 左下 `Caches` → 删除指定 key。
或在 yml 里改 cache key（如加版本号 `key: v2-...`）。

### Q7: workflow 历史怎么删

Actions 页面 → 左侧 workflow 名 → 右上 `...` → `Delete all workflow runs`。

### Q8: 怎么不让 fork 跑我的 CI

```yaml
on:
  pull_request:
    branches: [master]
jobs:
  build:
    if: github.event.pull_request.head.repo.full_name == github.repository
```

只在内部分支的 PR 跑，外部 fork PR 不跑。

### Q9: matrix 想跑 4×3=12 个，但只想跑其中部分

```yaml
matrix:
  os: [linux, windows, macos]
  cfg: [debug, release, relwithdebinfo, minsizerel]
  exclude:
    - os: macos
      cfg: minsizerel    # 排除 macos × minsizerel
  include:
    - os: linux
      cfg: asan          # 额外加一个 linux × asan
```

### Q10: 想在某 step 失败时也继续跑

```yaml
- name: 可能失败的步骤
  continue-on-error: true
```

或者：

```yaml
- name: 总是要跑的清理
  if: always()
  run: cleanup.sh
```

---

## 总结

```
.github/workflows/ci.yml
        │
        ▼
push / PR 触发
        │
        ▼
matrix 自动复制成 N 个并行 job
        │
        ▼
每个 job 在干净虚拟机里跑：
   actions/checkout → 装编译器 → 装 vcpkg → cmake → build → test
        │
        ▼
全绿 → 允许合并；红 → 拦截合并
```

**CI 一旦配好，开发者就该忘了它的存在**——它只在出错时跳出来提醒你。本项目这套配置专为支持"3 编译器多平台"目标而设计，是当前框架最关键的"质量保险栓"。

需要进一步扩展的方向：
- 加测试覆盖率上传（codecov）
- 加 sanitizer job（开 ASan/UBSan 跑测试）
- 加 release workflow（打 tag 自动出二进制）
- 加 nightly job（每晚跑一次 vcpkg head 跟踪上游变更）

把这套机制内化，下次你给任何 C++ 项目做 CI 都能直接复用。
