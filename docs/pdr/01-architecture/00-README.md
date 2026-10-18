# 架构：Design / Module / Hierarchy

本模块描述 DB 的设计聚合模型、逻辑层次和物理扩展边界。目标态以 **Design 为完整设计与持久化根**：一个 Design 包含一个 topmodule、其展开后的 module 表、层次树、名称池、库引用、约束和物理视图；没有 Design 实例/嵌套。

## 文档

| 文档 | 内容 |
| :--- | :--- |
| [01-spec.md](01-spec.md) | 实体、归属、定义-出现模型与不变量 |
| [02-interfaces.md](02-interfaces.md) | Design/current_design、库、SDC、物理阶段的概念接口 |
| [03-nfr.md](03-nfr.md) | 多 Design、持久化、ID/名称作用域、展开/打平约束 |
| [design/hierarchy.md](design/hierarchy.md) | topmodule 根、展开、HInst 树与 PathName |
| [design/physical.md](design/physical.md) | Inst → LEF Macro、Net 物理/寄生、HNet logic-only |

## 核心约束

- **Design 是完整设计，也是持久化单元**；`load` 得到一个完整 Design，不是 Design 的实例。
- **只有 Module 可以被实例化，且 Module 实例化永远是 `HInst`；`Inst` 是 Module 内部不可展开的叶实例（目标 LibCell）。** Design 没有实例化概念。
- `TopModule` 是 Design 的层次根；Design 持有顶层 HInst 列表，但没有额外 root HInst。
- `HInst::m_module_id` 永远指向 topmodule；`m_template_id` 指向实例化的 Module 定义。HInst 管理 child HInst、内部 Inst/HNet、边界 HPin；未展开时内部链表为空。
- `GetDesign`/`GetModule`/`GetHInst` 在创建/展开时赋值：`GetModule` 返回 topmodule，`GetHInst` 返回所属/上层 HInst（顶层 HInst 与 topmodule 直属对象为 null），查询不沿父链循环。
- Module 默认是跨 Design 共享的只读 base 定义；同一个 Module 可以是 A 的 top、B 的 helper。基础 hierarchy expansion 不做 uniquify，HInst 默认引用 base；需要私有修改时由 HInst uniquify 生成 unique 副本；unique Module 保留原始 Module 的 Design 成员，不新建 Design。Design 不维护 helper Module 表，从 topmodule 沿 HInst 树遍历全部层次。Module 不持有 HInst list/view，也不作为 hierarchy 的一层。
- 库（TechLib/TimingLib）以跨 Design 的只读 base 共享，按需通过 uniquify 创建 Design 私有副本；SDC 归 Design，支持 MCMM。
- PathName 由 HInst 父链按需合成，不存 flat path。
- `Net` 是 topmodule 级唯一逻辑网络，**只连接 Pin**，不能连接 HPin；`Pin` 可同时连接 `Net` 和 `HNet`；`HNet` 在所属 HInst 层级连接该层所有 `Pin` 和 `HPin`，用于表达本层次的完整连接。Net 每穿过一个 HInst，在该 HInst 内创建恰好一条代理 `HNet`。物理数据在 Net，HNet 只做逻辑层次代理。
- 物理阶段要求展开/打平 topmodule；`Inst` 的 ref 扩展 LEF Macro。

## 阅读顺序

先读 `01-spec` 建立 Design/Module/Inst/HInst 的定义与归属，再读 `02-interfaces` 理解当前设计与库/约束/物理边界，最后读 `03-nfr` 和 `design/` 下的层次/物理机制。
