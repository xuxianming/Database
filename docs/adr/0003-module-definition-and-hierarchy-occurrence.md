# Module 是定义，Inst/HInst 是两类出现

Module 是跨 Design 共享的只读 base 逻辑定义；`Inst` 是 Module 内部不可展开的叶实例（目标 LibCell），`HInst` 是 Module 实例化的唯一层次出现节点（Module 实例化永远产生 HInst）。TopModule 是 Design 的层次根；Design 持有顶层 HInst 列表，但没有额外的 root HInst。执行 hierarchy expansion 时在当前 Design 内建立 HInst 树及 helper Module 引用，**Module 不持有 HInst list/view，也不作为 hierarchy 中的一层**；Design 不维护 helper Module 表；unique Module 仍归原始 Design。PathName 从 topmodule 沿 HInst 父链合成。

`HInst::m_module_id` 永远指向 topmodule，由展开时设置；`m_template_id` 指向该 HInst 实例化的 Module 定义；`m_module_id`/`m_template_id` 可以引用共享 base Module，基础展开不做 uniquify。需要设计私有修改时，HInst 对 Module 执行 uniquify，生成 unique 副本；unique Module 保留原始 Design、不新建 Design。parent/children/sibling 由 HInst 树维护；`HInst::GetModule()` 直接返回 `m_module_id`，不循环；Module 不提供 `GetModule`。

理由：定义与出现分离后，同一个 Module 可以在一个 Design 内被多个 HInst 复用，也可以在多个 Design 中作为 top/helper；路径、层次边界和物理 occurrence 只属于出现的节点。`Inst` 不应被误用为层次树节点；Module 也不应承担 hierarchy 容器职责。

代价：Module 无唯一 PathName，路径查询需要 HInst 父链合成；不能把 per-occurrence 数据写回共享 base 定义；uniquify 才产生 unique Module 副本，且 unique 保留原始 Design。

## Considered Options

- **Module 直接作为层次节点**：无法区分同一 Module 的多次出现，PathName 和物理 occurrence 会冲突，未采用。
- **为 Design 增加一个 root HInst**：当前以 topmodule 为根，顶层 HInst 已能直接持有 topmodule id，额外 root 节点没有语义收益，未采用。
- **Module 持有 HInst list/view/unfold**：会把定义和层次容器混在一起，未采用。
- **基础展开就复制 Module**：内存/加载成本高且不利于共享，未采用；默认共享 base，按需 uniquify。

## Log

- 2026-10-19 修正 unique Module 归属：保留原始 Design，不新建 Design；补充统一导航字段。
- 2026-10-19 统一导航：`HInst::GetTopModule` 改为 `GetModule`，`GetUpperHInst` 改为 `GetHInst`。
