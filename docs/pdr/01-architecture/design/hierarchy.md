# 层次与 PathName

## 1. 根与展开

- Design 的层次根是 `top_module` 本身；没有额外 top HInst。
- 初始 Design 只有 topmodule。执行 hierarchy expansion 时，永远以当前 Module 为 template：
  1. 在 Design 的 top HInst 列表中创建顶层 HInst；每个 HInst 的 `m_module_id` 直接设为 topmodule；
  2. 基于 template Module 递归创建 child HInst，并连接 HInst 父子/sibling；
  3. HInst 管理内部 Inst、HNet、边界 HPin 和上下层连接；未展开时这些内部链表为空。
- Module 定义归属其原始 Design；展开不会引用另一个 Design 的可变对象。Module 不持有 HInst list/view，也不参与层次展开。

## 2. HInst 树

- 每个 HInst 的 `m_module_id` 永远指向 topmodule；`m_template_id` 指向它实例化的 Module 定义；parent 为空的顶层 HInst 由 Design 的 top HInst 列表持有。
- HInst 持有 child HInst、内部 Inst、HNet、边界 HPin 的链表；`m_next_hinst_id` 是 sibling。未展开时内部链表为空。
- HInst 遍历从自身开始递归向下；`GetHInst()` 直接返回 parent（顶层为 null），`GetModule()` 直接返回 `m_module_id`。
- 同一个 Module 可以在一个 Design 内被多个 HInst 出现；Module 是共享定义，HInst 是独立 occurrence。
- 子 HInst 的集合表达 hierarchy tree；层次边界用 HNet/HPin 表达。
- `Inst` 与 `HInst` 的关系：Inst 是 Module 内部的叶实例；HInst 是展开后的层次出现，并管理该出现内的 Inst/HNet/HPin。Module 不持有 HInst。
- 统一导航：`Module`/`HInst`/`Inst`/`Net`/`HNet`/`HPin` 的 `GetDesign` 返回顶层 Design；`HInst`/`Inst`/`Net`/`HNet` 的 `GetModule` 返回 topmodule；`HInst`/`Inst`/`Net`/`HNet` 的 `GetHInst`：HInst 返回上层 HInst，其余返回所在 HInst；顶层 HInst 与 topmodule 直属对象为 null。

### Net ↔ HNet

- Net 是 Design/topmodule 级唯一逻辑网络，存储只在 topmodule 的 Net 表；它忽略层次，只连接 Pin，**不能连接 HPin**；ModulePort 通过 io Pin 连接。
- HNet 是 Net 的层次代理；owner 为 Module（顶层 HNet）或 HInst（中间 HNet），二者互斥。Net 每穿过一个 HInst，就在该 HInst 内创建恰好一条 HNet。
- 一个 Net 可以对应 0..N 个 HNet；每个 HNet 只对应一个 Net。Net 未穿层级时可以没有 HNet。
- HNet 在所属 HInst 层级连接该层所有 Pin 和 HPin；Pin 可同时连接 Net 和 HNet，因此 HNet 能表达本层次完整的连接关系。HNet 只承载逻辑层次信息，物理/寄生数据留在 Net。HNet::GetModule 返回所属层次的 topmodule，owner 字段只用于确定 GetHInst；HPin 不提供 GetModule。

## 3. PathName

- PathName 从 topmodule 名开始，沿 HInst 父链拼接：`top/u0/u1`。
- 分隔符和转义规则由实现统一；不在 Module 上保存 flat path。
- Module 定义本身没有唯一 PathName；只有 HInst（或 view）有路径。
- `HInst::m_path_name_id` 当前未使用，目标模型中不应作为 flat path 主存储；如需缓存，只做可失效的派生缓存。
- 顶层导航：`HInst::GetModule()` 直接返回 `m_module_id`（topmodule），不循环；Module 不提供 top 导航，也不参与 hierarchy 展开。

## 4. helper Module 与另一个 Design 的 top

- 同一个逻辑 Module 在 Design A 中是 helper，在 Design B 中可以是 top；默认共享只读 base，HInst 引用该 base。
- 需要设计私有修改时，HInst 可 uniquify 为 unique Module；unique Module 的 Design 成员保持为原始 Module 的 Design，不新建 Design。可变设计对象不跨 Design 共享。
- PathName 不需要跨 Design 组合；每个 Design 以自己的 topmodule 为根，沿 HInst 树遍历 helper/unique Modules。

## 5. 物理阶段

- 进入 physical 前，topmodule 必须展开或打平。
- Net 在 Design/topmodule 级保持唯一并承载物理/寄生数据；多个 HNet 只是同一 Net 的层次表示，不复制 Net。
- HNet 仍只表达逻辑层次连接；细节见 [physical.md](physical.md)。
