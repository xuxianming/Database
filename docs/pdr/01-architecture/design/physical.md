# 物理扩展

## 1. 前置条件

- 物理阶段要求 topmodule 展开或打平；未展开的逻辑层次不足以建立 per-instance 的布局/物理视图。Net 的寄生数据仍归 topmodule 级唯一的 Net。
- 物理视图归 Design 所有；不引入跨 Design 的物理对象引用。

## 2. Inst → LEF Macro

- `Inst` 的逻辑 ref 指向 Module 或 LibCell；进入物理阶段后，其 ref 扩展/绑定 LEF Macro（TechLib 中的物理抽象）。
- LEF Macro 是共享库对象，只读；Design 通过引用使用。
- placement 等 per-instance 物理状态属于 Design 的物理视图，不应写回共享 LEF Macro。Net 的物理身份则保持在 topmodule 级唯一的 Net 上。

## 3. Net 与寄生

- `Net` 是 Design/topmodule 级唯一的逻辑网络，存储只在 topmodule 的 Net 表；物理信息与寄生参数直接由该 Net 承载。
- `Net` 可以忽略 hierarchy，直接连接 Pin；它不连接 HPin，也不因为跨层次而被复制。ModulePort 通过其 io Pin 连接。
- 一个 Net 可以对应 0..N 个 `HNet`（每个穿过的 HInst 恰好一条代理）；HNet 只是同一 Net 的层次代理，因此物理/寄生数据不需要按 HNet 复制，也不需要单独引入 PhysicalNet 作为主存储。
- 物理视图负责把 Net 的物理数据与 Inst/HInst、Net/HNet 的 occurrence 关系关联起来，支持后续 parasitics、routing、RC 等扩展。

## 4. HNet 的边界

- `HNet` 是 Net 在某个 HInst 内的代理，只表达逻辑层次信息，不承载 physical/parasitic 字段。
- 未展开 hierarchy 时，Net 可以没有 HNet（表示没有穿层级）；Net 每穿过一个 HInst，就在该 HInst 内创建恰好一条 HNet。
- 每个 HNet 唯一对应一个 Net；Net 可对应多个 HNet。HNet 连接 Pin 和 HPin，用于表达本层次完整连接；Net 不连接 HPin。
- 不允许把物理信息隐式复制到 HNet/Module 定义，避免层次表示与物理主数据分离后产生不一致。

## 5. 与 SDC/MCMM 的关系

- SDC/MCMM 约束归 Design；物理/寄生数据可作为 timing 分析的输入，但约束本身不写回共享库。
- corner/mode/scenario 的组合由 Design 管理；TimingLib 提供共享 corner 数据。
