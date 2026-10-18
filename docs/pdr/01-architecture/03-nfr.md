# 非功能约束

## 1. 多 Design 生命周期

- 多个 Design 可并存；每个 Design 是独立完整设计。
- `current_design` 是工具当前活动的 Design；大多数操作只作用于它。
- 实际流程通常只展开 current_design；其他 Design 保持完整但未展开。

## 2. 持久化

- 持久化以 Design 为单元；快照覆盖 topmodule、HInst 树、从 HInst 可达的 base/unique Modules、名称、SDC、物理视图，以及共享库版本/解析信息。
- `load` 应恢复一个完整 Design，不依赖另一个 Design 的内部可变对象；共享 base 只读依赖必须可解析。
- Design 不维护 helper Module 表；序列化按 HInst 树收集可达 Modules。uniquify 产生的 unique Module 保留原始 Design 成员，不新建 Design，但必须随可达 HInst 树一起持久化。
- 库是共享只读 base；快照必须包含库的版本/解析信息，保证完整设计可恢复。
- Design 私有修改通过 uniquify 创建副本；快照必须区分“共享 base 引用”和“unique 副本”，unique Module 通过 HInst 树可达并随 Design 快照保存（保留原始 Design 成员）。
- **待决**：库内容内嵌到 Design 快照，还是保存版本化外部依赖并在 load 时解析？两种方式都需保证“完整设计”语义。

## 3. ID 与名称作用域

- Design 私有的 Inst/Net/Name 属于 Design；其 ID/名称作用域应明确为 Design 内。
- base Module、unique Module 与共享库对象使用各自 base/unique 作用域的名称/ID，不能与 Design 私有名称池混用。
- HInst 必须能区分它引用的 Module 是 base 还是 unique 副本。
- PathName 按 HInst 父链合成；不将 flat path 作为持久化主键。

## 4. 展开/打平

- 展开会产生 helper Modules 与 HInst 树，内存随展开规模增长。
- 物理阶段要求 expand/flatten；Net 在 Design/topmodule 级唯一，HNet 是 0..N 个层次代理，物理数据不复制到 HNet。
- Net 不穿层级时可以没有 HNet；Net 每穿过一个 HInst，就在该 HInst 内创建恰好一条代理 HNet；每个 HNet 只对应一个 Net。
- Net 不能连接 HPin；Pin 可同时连接 Net 和 HNet；HNet 在所属 HInst 层级连接该层所有 Pin 和 HPin，保证本层次连接完整。
- 展开应可重复/可查询，不应隐式创建跨 Design 引用。

## 5. 兼容性与性能

- Design 快照应带版本；版本变更需可迁移或明确拒绝。
- 大量 Inst/Net 下，PathName 按需合成、避免 flat intern；名称池与对象表应支持 Design 级生命周期。
- 约束/库的共享不得导致 Design 之间的可变状态串扰。
