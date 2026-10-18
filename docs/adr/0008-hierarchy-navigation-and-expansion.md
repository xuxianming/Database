# 层次展开与统一导航字段

层次展开永远基于一个 Module：以该 Module 为 template，递归创建 HInst。HInst 是层次容器，管理内部实例化的 child HInst、Inst、HNet，以及边界 HPin 和上下层连接；未展开的 HInst 内部链表为空。Module 只保留定义（含叶 Inst），不持有 HInst list/view；从 topmodule 的顶层 HInst 开始递归遍历可访问全部层次。

所有逻辑对象在创建/展开时写入导航字段，查询直接返回缓存值，禁止沿 parent 链循环：

- `Module`/`HInst`/`Inst`/`Net`/`HNet`/`HPin` 提供 `GetDesign`，返回顶层 Design；`Module` 返回其原始 Design，unique 副本保留原 Design 成员。
- `HInst`/`Inst`/`Net`/`HNet` 提供 `GetModule`，返回 topmodule；HInst 用 `m_module_id`，其余对象用展开时写入的 topmodule id。
- `HInst`/`Inst`/`Net`/`HNet` 提供 `GetHInst`：`HInst` 返回上层 HInst，`Inst`/`Net`/`HNet` 返回所在 HInst；顶层 HInst 与 topmodule 直属对象返回 null。
- unique Module 仍归原始 Module 的 Design 管理；使用它的 Design 不新建 Design，也不维护 helper Module 表。

理由：层次对象的 owner/template Module 与 topmodule 不是同一个概念；把 topmodule、Design、HInst 直接缓存在对象上，查询 O(1) 且不依赖父链，适合大量 occurrence 的展开和物理流程。HInst 作为内部对象容器也能把同一 Module 的多次 occurrence 数据隔离开。

代价：每个对象增加导航字段，展开时必须完整赋值并保持一致性；HInst 内部 Inst/HNet 需要按 occurrence 管理（或明确 clone/view 策略），增加内存和持久化复杂度。
