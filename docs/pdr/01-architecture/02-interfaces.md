# 概念接口

本文件描述对外部工具/模块暴露的概念接口，不绑定具体 C++ 签名。

## 1. Design 与 current_design

- 选择/加载一个 Design 后，`current_design` 指向该完整设计。
- `load` 返回完整 Design：topmodule、HInst 树、从 HInst 可达的 Modules、名称、约束、物理视图（若存在）和库引用；不要求 helper Module 表。
- 多 Design 可以并存，但任一工具实例通常只展开/优化 `current_design`。
- 不提供 Design 实例化 API；不存在 Design 之间的 parent/child 关系。

## 2. Module / Inst / HInst

- 查询/创建 Design 的 topmodule；第一个创建的 Module 默认成为 topmodule，可用 `SetTopModule` 覆盖。
- Module 默认引用共享 base 定义；HInst 可对 base Module 执行 **uniquify**，生成 unique 副本；unique Module 的 Design 成员仍为原始 Module 的 Design，不新建 Design。
- 基础 hierarchy expansion 不 uniquify；Design 不维护 helper Module 表，从 topmodule 沿 HInst 树遍历全部层次；unique Module 由原始 Design 管理。
- 在 Module 内创建 `Inst`，目标是 LibCell；Inst 是 Module 内部叶实例，不可展开。
- Module 实例化永远创建 HInst。执行 hierarchy expansion：在当前 Design 内创建顶层 HInst、子 HInst 和边界连接；顶层 HInst 由 Design 的 top HInst 列表持有。
- 查询 HInst 的 parent/children/sibling；生成/查询 PathName。
- `HInst::GetModule()` 直接返回 `m_module_id`（topmodule），不做父链循环；parent 由 `GetHInst()` 直接返回。
- 统一导航接口：`Module`/`HInst`/`Inst`/`Net`/`HNet`/`HPin` 提供 `GetDesign`；`HInst`/`Inst`/`Net`/`HNet` 提供 `GetModule`，返回 topmodule；`HInst`/`Inst`/`Net`/`HNet` 提供 `GetHInst`：HInst 返回上层 HInst，其余返回所在 HInst；顶层 HInst 与 topmodule 直属对象为 null。
- `GetDesign`/`GetModule`/`GetHInst` 在创建/层次展开时赋值，直接返回，不沿 parent 链循环。
- `Module` 不提供顶层接口，也不持有 HInst；`Design` 拥有 topmodule 和顶层 HInst 列表。
- `Inst` 和 `HInst` 不可混用：前者是逻辑实例，后者是层次出现；HInst 管理内部 Inst/HNet/HPin，未展开时内部链表为空。

## 3. 库

- TechLib/TimingLib 由共享 LibraryManager 提供，Design 通过引用使用只读 base。
- 提供 **uniquify** 接口：需要设计私有修改时，创建 Design 私有副本；副本归 Design 所有，可修改/持久化。
- 共享 base 不可被 Design 直接修改；uniquify 后对象与 base 的身份/版本关系必须可追踪。
- Design 快照需要记录所用库的版本/解析信息；具体“内嵌库快照”还是“版本化外部依赖”见 NFR 的待决项。

## 4. SDC / MCMM

- Design 拥有一个或多个 SDC/ConstraintSet。
- MCMM 由 mode/corner/scenario 组合描述；TimingLib 提供共享 corner 数据。
- 约束引用 Design 内对象或 PathName；约束的生命周期随 Design。

## 5. 物理阶段

- 进入物理阶段前调用 expand/flatten topmodule。
- `Inst` 的 ref 扩展为 LEF Macro；物理布局/布线/寄生数据归 Design 的物理视图。
- `Net` 是 Design/topmodule 级唯一逻辑网络，也是物理/寄生信息的唯一载体；Net 只连接 Pin，不连接 HPin；ModulePort 通过其 io Pin 连接。
- `HNet` 是 Net 的层次代理；owner 为 Module（顶层 HNet）或 HInst（中间 HNet），二者互斥。每个穿过的 HInst 恰好一条 HNet，每个 HNet 只对应一个 Net；HNet 在所属 HInst 层级连接该层所有 Pin 和 HPin，只承载逻辑层次信息。
- `HNet::GetModule()` 返回所属层次的 topmodule；owner 字段只用于确定 `GetHInst()`。`Pin` 可同时连接 `Net` 和 `HNet`；`HPin` 不提供 `GetModule()`。
- 物理视图与逻辑视图的关联通过 `Net`（物理）↔ 多个 `HNet`（按 HInst 的层次代理）以及 Inst/HInst 建立；不能把物理字段写到 HNet 上。
