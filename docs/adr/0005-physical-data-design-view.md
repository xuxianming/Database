# 物理数据在顶层唯一 Net 上，HNet 仅层次表示

物理阶段要求 topmodule 展开或打平。`Inst` 的 ref 在物理阶段扩展/绑定到 LEF Macro；`Net` 是 Design/topmodule 级唯一的逻辑网络，存储只在 topmodule 的 Net 表，物理信息和寄生参数直接由该 Net 承载。`HNet` 是 Net 在某个 HInst 内的唯一代理：Net 每穿过一个 HInst 创建恰好一条 HNet，每个 HNet 唯一对应一个 Net；HNet 连接 Pin 和 HPin，只承载逻辑层次信息，不承载 physical/parasitic 字段。Net 只连接 Pin，不连接 HPin。

理由：Net 保持顶层唯一后，物理/寄生数据不需要按 HNet 复制；HNet 只解释同一 Net 在层次中的表示，避免多个 HNet 之间出现物理主数据不一致。

代价：需要维护 Net ↔ 多个 HNet 的映射；HNet 不能作为物理数据入口；物理阶段仍需展开/打平 topmodule 以建立 Inst/HInst 的物理视图。

## Considered Options

- **每个 HNet 一份物理数据**：同一 Net 会被复制成多个物理网络，难以保持一致，未采用。
- **把物理字段挂在 HNet 上**：HNet 是层次表示而非物理网络，未采用。

## Log

- 2026-10-18 按 Net/HNet 层次表示规则修订物理数据归属。
- 2026-10-19 修正连接模型：Net 只连 Pin；HNet 连 Pin 和 HPin。
