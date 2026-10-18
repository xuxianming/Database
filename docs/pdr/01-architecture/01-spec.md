# Design 与逻辑对象模型

## 1. 范围

本规范定义 DB 的聚合根、逻辑对象和层次展开模型。目标态以 Design 为中心，不描述具体文件格式或类实现细节。

## 2. Design：完整设计与持久化根

**Design** 是一个完整用户设计的设计上下文，管理该设计的全部私有信息：

- `top_module`：本 Design 的顶层 Module，也是层次根和入口；
- topmodule 持有顶层 HInst 列表；Design 本身不维护 HInst 链表；从 topmodule 出发递归遍历全部 helper Module/出现；
- 不维护 helper Module 表；helper/base/unique Module 通过 HInst 的 `template` 引用访问。unique Module 仍归其原始 Design。
- `StringMgr` / `IdArrayMgr` 与对象名称；
- 逻辑对象（Inst/Net/Pin/Port 等）；
- SDC / MCMM 约束；
- 对共享 TechLib/TimingLib 的引用；
- 物理视图（展开/打平后）。

约束：

- **Design 没有实例化/嵌套概念**。不存在“Design 里的 Design”，也不存在跨 Design 的可变设计对象引用；只读 base Module/库可以共享。Design 只持有 topmodule，不持有 HInst 链表。
- 一个 Design 的持久化快照必须能够恢复该 Design 的完整设计私有状态；`load` 的结果是一个完整 Design。
- 多 Design 可以并存，但工具以 `current_design` 为当前活动设计；实际流程通常只展开 current_design。

## 3. Module：Design 内的逻辑定义

**Module** 是一个逻辑定义/容器，包含：

- Module ports / logical pins；
- 子 `Inst`（逻辑实例）。

Module 定义本身不持有 HInst tree，也不作为层次节点；当它是 Design 的 topmodule 时，额外持有顶层 HInst 列表，并作为层次入口递归处理 HInst 树。

`Net` 不是 Module 私有对象：Design/topmodule 维护**全局唯一的 Net 表**。`HNet` 的 owner 可以是 Module（顶层 HNet）或 HInst（中间 HNet）；`HPin` 归 HInst。Module 定义不持有 HInst tree；topmodule 持有顶层 HInst 列表并递归处理层次。

归属与身份：

- Module 默认是**跨 Design 共享的只读 base 定义**；同一个 Module 可以是 Design A 的 topmodule，同时是 Design B 的 helper module。
- 基础 hierarchy expansion **不做 uniquify**：HInst 默认引用共享 base Module。
- 需要设计私有修改时，HInst 可以 **uniquify** 该 Module，生成 unique 副本；**unique Module 的 Design 成员仍是原始 Module 的 Design，不新建 Design**。它由原始 Design 管理，并被 HInst 的 template 引用。
- Design 不维护 helper Module 表；从 topmodule 沿 HInst 树遍历即可访问全部 helper/unique Modules。
- Module 本身没有唯一 PathName；PathName 属于它在层次中的出现（HInst）。
- Module 持有原始 Design 成员；unique 副本保留该成员，不因被其他 Design 的 HInst 引用而改属。名称/ID 作用域按原始 Design + Module 定义解析。
- **Module 没有顶层导航接口**；`HInst::m_module_id` 永远指向 topmodule，`HInst::GetModule()` 直接返回，不做父链循环。Design 保留 topmodule 根。

## 4. Inst 与 HInst：定义 vs 出现

| 对象 | 层级 | 说明 |
| :--- | :--- | :--- |
| `Inst` | 逻辑叶实例 | Module 内部声明、不可展开的实例；目标是 LibCell；有局部名 |
| `HInst` | 展开出现 | Module 实例化的唯一层次节点；`m_module_id` 永远指向 topmodule，`m_template_id` 指向实例化的 Module 定义；有 parent/children/sibling；唯一 PathName |

- `Inst` 不是层次树节点，也不能展开；它表达 Module 内部的叶实例。Module 实例化永远产生 HInst，不会产生“可展开的 Inst”。
- `HInst` 是 occurrence；同一个 Module 可以被多个 HInst 出现，路径只属于 HInst。
- `HInst::GetModule()` 直接返回 `m_module_id`（topmodule），不做父链循环；`HInst::GetHInst()` 直接返回 parent。
- 顶层 HInst（parent 为空）由 topmodule 的顶层 HInst 列表持有；每层 HInst 持有自己的 child list。Design 不持有 HInst 链表。
- `Pin` 同时保存 `m_net_id` 和 `m_hnet_id` 两个连接；`Net` 只连接 Pin（不能直接连接 HPin；ModulePort 通过其 io Pin 连接），`HNet` 在所属 HInst 层级连接该层的所有 Pin 和 HPin。HNet 因此能表达本层次的完整连接。
- `HNet` 的 owner 为 Module 或 HInst（二选一）；HPin 归 HInst；Net 通过 HNet 代理连接到 HPin。Pin 同时保留 Net（全局连接）和 HNet（层次内连接）。

### Net 与 HNet

| 对象 | 归属 / 基数 | 说明 |
| :--- | :--- | :--- |
| `Net` | Design/topmodule，全局唯一 | 一条逻辑网络；存储只在 topmodule 的 Net 表；承载 physical/parasitic 信息；**只连接 Pin**，不连接 HPin；ModulePort 通过 io Pin 连接 |
| `HNet` | Module 或 HInst（二选一），0..N / Net（每个穿过的 HInst 恰好一条） | Net 的层次代理；owner 为 Module 时是顶层 module HNet，owner 为 HInst 时是展开产生的中间 HNet；唯一对应一个 Net；在其 HInst 层级连接该层所有 Pin 和 HPin；不承载 physical/parasitic |

- Net 是物理信息的唯一载体；HNet 只是 Net 的层次代理，不承载 physical/parasitic 字段。
- HNet 的 owner 是 **Module xor HInst**，不能同时存在：
  - owner 为 Module：顶层 module 的 HNet；
  - owner 为 HInst（module 为空）：该 HInst 内、由 hierarchy 展开产生的中间 HNet。
- HNet 提供 `GetModule()`：返回该 HNet 所属层次的 topmodule；owner 字段只决定 HNet 属于顶层 Module 还是某个 HInst。HPin 不提供 `GetModule()`。
- 初始未展开 hierarchy 时，Net 可以没有 HNet，表示这条 Net 没有穿层级；Net 每穿过一个 HInst，就在该 HInst 内创建**恰好一条**代理 HNet。
- 一个 Net 可以对应多个 HNet（每个穿过的 HInst 一条）；HNet 不构成新的逻辑网络。
- Net 不能连接 HPin；Pin 可同时连接 Net 和 HNet，HNet 负责本层次内 Pin/HPin 的完整连接。

## 5. 展开与 TopModule

- `TopModule` 是 Design 的逻辑根和入口；Design 没有额外的 root HInst，也不持有 HInst 链表。
- hierarchy expansion 永远基于一个 Module：以该 Module 为 template，递归创建 HInst；HInst 管理内部实例化的 child HInst、Inst、HNet，以及边界 HPin 和上下层链接。
- 初始时 Design 只有 topmodule；展开时才在本 Design 内创建顶层 HInst、子 HInst 和边界对象。
- 顶层 HInst 由 topmodule 的顶层 HInst 列表持有；`HInst::m_module_id` 在创建时设为 topmodule，`m_template_id` 指向实例化 Module。
- 未展开的 HInst 内部链表为空；此时 Module 只能遍历其叶 Inst 定义。展开后从 HInst 自身开始递归向下遍历 child HInst。
- PathName 以 topmodule 名为根，例如 `top/u0/u1`；由 HInst 父链合成。
- 进入物理阶段前，topmodule 必须展开或打平；物理数据只在展开/打平后的 Design 视图中有意义。

## 6. 库与约束

- **TechLib / TimingLib** 以跨 Design 共享的只读 base 形式提供；Design 默认持有引用，不复制。
- Design 需要设计私有修改时，通过 **uniquify** 接口创建 Design 私有副本；uniquify 后对象归 Design 所有，可修改并随 Design 持久化。
- **SDC** 归 Design 所有，支持 MCMM（mode/corner/scenario）；约束引用 Design 内的对象或 PathName。
- 共享库的名称属于库作用域，不与 Design 私有名称池混用。

## 7. 物理扩展（摘要）

- `Inst` 的 ref 在物理阶段扩展为 LEF Macro 的引用。
- `Net` 是顶层唯一逻辑网络，承载该 Design 的物理信息和寄生参数；`HNet` 只做逻辑层次表示。
- 物理阶段要求 topmodule 展开/打平，以便建立 Inst/Net 的物理视图；Net 的身份仍保持在 Design/topmodule 级，不因多个 HNet 而复制。
- 详见 [design/physical.md](design/physical.md)。

## 8. 统一导航接口

所有逻辑对象在创建/层次展开时缓存导航字段；查询直接返回缓存值，不沿 parent 链循环。

| 对象 | GetDesign | GetModule | GetHInst |
| :--- | :--- | :--- | :--- |
| `Module` | 原始/所属 Design；unique 保留原 Design | — | — |
| `HInst` | 顶层 Design | topmodule（`m_module_id`） | 上层 HInst；顶层 HInst 为 null |
| `Inst` | 顶层 Design | topmodule（展开时赋值） | 所属 HInst；topmodule 直属为 null |
| `Net` | 顶层 Design | topmodule | 所属 HInst；topmodule 级为 null |
| `HNet` | 顶层 Design | topmodule（展开时赋值） | 所属 HInst；顶层 HNet 为 null |
| `HPin` | 顶层 Design | — | 所属 HInst |

- `GetModule()` 统一返回顶层 Module，不返回 Inst 的 owner Module 或 HNet 的 template/owner Module；HInst 用 `m_module_id`，Inst/Net/HNet 用展开时写入的 topmodule 字段。
- `GetHInst()` 统一返回对象所属的层次 HInst：`HInst` 返回上层 HInst，`Inst`/`Net`/`HNet` 返回所在 HInst；顶层 HInst 和 topmodule 直属对象返回 null。
- `GetDesign()` 统一返回顶层 Design；`Module::GetDesign()` 返回其原始 Design，unique 副本不因被其他 Design 的 HInst 引用而改变。
- HInst 内部持有 child HInst、内部 Inst、HNet 和边界 HPin 的链表；未展开时这些链表为空。Module 的叶 Inst 定义可直接遍历；层次遍历从 topmodule 的顶层 HInst 开始递归。

## 9. 不变量

1. Module base 可跨 Design 共享只读；HInst 默认引用 base，必要时 uniquify 为 unique 副本；unique Module 保留原始 Design，不新建 Design；使用它的 helper Design 不管理 helper Module 表。
2. 不存在 Design 实例；只有 Module 可实例化，且 Module 实例化永远是 HInst。Inst 是 Module 内部的叶实例，不可展开。
3. Design 只管理 topmodule；topmodule 持有顶层 HInst 列表并递归处理 HInst 树，Design 不持有 HInst 链表。
4. TopModule 是 Design 的根；HInst 树从 topmodule 的顶层 HInst 向下展开，child HInst 由 HInst 持有。
5. Module 无唯一路径；PathName 只由 HInst 父链合成。
6. HInst 的 `m_module_id` 永远是 topmodule；`m_template_id` 是该 HInst 实例化的 Module。
7. Net 在 Design/topmodule 级唯一；HNet 是 0..N 个层次表示，每个 HNet 唯一对应一个 Net。
8. 物理阶段要求展开/打平；物理数据在 Net 上，HNet 永远只表达逻辑层次信息。
9. Module/HInst/Inst/Net/HNet/HPin 的 `GetDesign`、`GetModule`、`GetHInst` 在创建/展开时赋值，禁止沿父链循环推导。
