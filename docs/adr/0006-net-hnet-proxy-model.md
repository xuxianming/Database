# Net 在 HInst 中以唯一代理 HNet 表示层次

`Net` 是 Design/topmodule 级唯一的逻辑网络，存储只在 topmodule 的 Net 表：它只能直接连接 Pin，**不能连接 HPin**。当 Net 穿过一个 HInst 时，在该 HInst 内创建**恰好一条代理 HNet**；HNet 唯一对应这个 Net，并在该 HInst 层级连接本层所有 Pin 和 HPin，以表达本层次的完整连接。Net 不穿层级时可以没有 HNet；一个 Net 可对应 0..N 个 HNet（每个穿过的 HInst 一条）。

HNet 的 owner 是 **Module 或 HInst，二者互斥**：owner 为 Module 时是顶层 module HNet；owner 为 HInst（module 为空）时是 hierarchy 展开产生的中间 HNet。HNet 提供 `GetModule()`（返回所属层次的 topmodule，owner 字段只用于确定 `GetHInst()`）；HPin 不提供 `GetModule()`。

理由：Net 保持顶层唯一，物理/寄生数据不需要按 HNet 复制；HNet 只解释同一 Net 在某个 HInst 内的层次代理，避免一个 HInst 内出现多条互相竞争的网络表示。

代价：需要维护 Net 与多个 HNet 的一对多映射，并保证每个穿过的 HInst 恰好一条 HNet；Pin 同时维护 Net/HNet 两套连接；HNet 不能作为物理数据入口；Net→HPin 必须被禁止，HPin 只能通过 HNet 接入。

## Considered Options

- **一个 HInst 内多条 HNet**：同一 Net 在相同层次出现内会有多个表示，语义不唯一，未采用。
- **Net 直接连接 HPin**：跳过层次代理，破坏 Net/HNet 的职责分离，未采用。
- **Net 在每个 Module 边界保存 HNet**：与“按 HInst 代理”不一致，未采用。

## Log

- 2026-10-19 `HNet::GetModule()` 语义改为返回 topmodule；owner 字段保留用于层次代理与 `GetHInst()`。
- 2026-10-19 修正连接模型：Net 只连 Pin；Pin 可同时连 Net/HNet；HNet 连 Pin 和 HPin。
