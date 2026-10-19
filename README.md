# Database

C++20 的 EDA 网表数据库，当前聚焦 **logic DB**：Design/Module 层次、Inst/Net/Pin 逻辑连接，以及后续物理流程所需的数据库边界。

项目目标是把“设计定义”“层次出现”“逻辑网络”“层次代理”和“持久化”分开建模，避免用一个 Module 或一个 HInst 同时承担所有职责。

## 当前状态

已完成并验证：

- `Design`：完整设计、`current_design`、topmodule、名称池、对象表边界；
- `Module`：逻辑定义、Inst 创建、base/unique 基础；
- `HInst`：层次 occurrence、parent/child/sibling、PathName；
- `Inst`：LibCell 叶实例、Pin 数组；
- `Net` / `Pin` / `HNet` / `HPin`：顶层连接与层次代理的分工；
- `StringMgr`、`ObjectTable`、`IdArrayMgr`、Save/Load 基础设施；
- CMake shared library、install/export package（`Database::database`）；
- DFG 核心对象（DFGGraph/DFGPrim/DFGNode/DFGPin/DFGEdge）及 Design 挂载、内存表和 Save/Load；
- GoogleTest 单测：当前 41 个测试。

当前仍属于后续工作的部分：

- 基于 Module 的递归 hierarchy expansion；
- 每个 HInst occurrence 的 Inst/HNet 克隆与自动填充；
- 展开时自动把本层所有 Pin/HPin 挂接到 HNet；
- `HInst::UniquifyModule` 的完整对象复制；
- 物理视图、LEF Macro、寄生数据。

设计目标与实现差异见 [DESIGN.md](DESIGN.md) 和 [doc/README.md](doc/README.md)。

## 核心设计

### Design 是根

`Design` 是一份完整设计，也是持久化单元：

- 拥有 topmodule；
- **不持有 HInst 链表**；顶层 HInst 列表由 topmodule 持有；
- 不维护 helper Module 表；
- 不存在 Design 的实例/嵌套。

### Module 是定义，HInst 是出现

- `Module` 是逻辑定义/容器，包含 ports 和 Inst 定义；
- `HInst` 是 Module 实例化的层次 occurrence；
- 同一个 Module 可以被多个 HInst 出现；
- `HInst::m_module_id` 永远指向 topmodule；
- `HInst::m_template_id` 指向该 occurrence 的 Module 定义。

### Inst 是叶实例

`Inst` 目标为 `LibCell`，不可展开。Module 实例化永远产生 HInst，不会产生“可展开的 Inst”。

### Net/Pin 与 HNet/HPin

- `Net` 是 topmodule 级唯一逻辑网络，**只连接 Pin**；
- `Pin` 同时保存 `m_net_id` 和 `m_hnet_id`；
- `HNet` 是 Net 在某个 HInst 层级的代理，连接该层所有 Pin 和 HPin；
- `HPin` 是层次边界，不直接连接 Net；
- 物理/寄生数据在 Net，HNet 只承载逻辑层次信息。

### 统一导航

对象在创建/展开时缓存导航字段，查询不沿父链循环：

| 对象 | `GetDesign()` | `GetModule()` | `GetHInst()` |
| :--- | :--- | :--- | :--- |
| `Module` | 原始 Design | — | — |
| `HInst` | 顶层 Design | topmodule | 上层 HInst；顶层 null |
| `Inst` | 顶层 Design | topmodule | 所在 HInst；直属 topmodule 为 null |
| `Net` | 顶层 Design | topmodule | 所在 HInst；顶层 null |
| `HNet` | 顶层 Design | topmodule | 所属 HInst；顶层 HNet null |
| `HPin` | 顶层 Design | — | 所属 HInst |

## 构建

### 环境要求

- CMake >= 3.20
- C++20 编译器
- GoogleTest（构建测试时需要）
- `slang`（构建 `my_tool` demo 时需要）

### 使用构建脚本

默认构建并安装：

```bash
./build.sh
```

清理构建目录、构建但不安装：

```bash
./build.sh -c
```

构建测试：

```bash
./build.sh -c -t
ctest --test-dir build --output-on-failure
```

指定构建目录、构建类型、并行度：

```bash
./build.sh -b build_debug -T Debug -j 8 -c
```

### 直接使用 CMake

```bash
cmake -S . -B build \
  -DDB_BUILD_TOOL=OFF \
  -DDB_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

`DB_BUILD_TOOL` 默认 `ON`，会构建依赖 `slang` 的 `my_tool` demo；只使用库时建议设为 `OFF`。
`DB_BUILD_TESTS` 默认 `OFF`。

### 安装与外部使用

```bash
cmake -S . -B build \
  -DDB_BUILD_TOOL=OFF \
  -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build --parallel
cmake --install build
```

外部 CMake 工程：

```cmake
find_package(Database CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE Database::database)
```

## 目录结构

```text
include/       公共头文件
src/           实现
test/          GoogleTest 单测（单一 db_ut）
doc/           Logic DB 类级设计文档
docs/          PDR、ADR、issue
DESIGN.md      Design-centric 设计总览
CONTEXT.md     术语表
```

## 文档

- [Design 总览](DESIGN.md)
- [Logic DB 类级文档索引](doc/README.md)
- [Logic DB 设计理念](doc/00-overview.md)
- [Slang RTL 导入设计](doc/SlangImport.md)
- [DFG RTL/Logic 建模](doc/DFG.md)
- [PDR 01 架构](docs/pdr/01-architecture/00-README.md)
- [对象模型规范](docs/pdr/01-architecture/01-spec.md)
- [概念接口](docs/pdr/01-architecture/02-interfaces.md)
- [非功能约束](docs/pdr/01-architecture/03-nfr.md)
- [术语表](CONTEXT.md)
- [ADR 目录](docs/adr/README.md)

## 验证

当前仓库的验证命令：

```bash
./build.sh -c
./build.sh -c -t
ctest --test-dir build --output-on-failure
```

最近一次验证结果：41/41 测试通过，默认构建通过，ASan/UBSan 41/41 通过。

## License

本仓库当前未包含独立 LICENSE 文件；使用/分发前请先与仓库 owner 确认授权方式。
