# Design 是完整设计与持久化根

Design 表示一个完整用户设计，管理 topmodule、顶层 HInst 树、名称池、SDC/MCMM、库引用和物理视图；**不维护 helper Module 表**，unique Module 仍归其原始 Design。HInst 树从 topmodule 遍历全部 helper/unique Modules。Design 没有实例化/嵌套概念；`current_design` 只是当前活动设计。持久化以 Design 为单元，`load` 得到一个完整 Design；共享只读 base（Module/库）通过引用/版本信息参与恢复，unique Modules 通过 HInst 树可达并随快照保存。

理由：上层工具按需选择并展开一个 Design；Design 必须拥有完整、可持久化的设计私有状态，同时允许只读 base 被多个 Design 复用。可变设计对象跨 Design 共享会让所有权、名称/ID 作用域和持久化边界失控。

代价：Design 之间不共享可变 Module/Net/Inst 对象；共享 base Module/库需要版本/解析信息；uniquify 后的 Module 副本会增加内存/磁盘占用。

## Considered Options

- **Design 作为可实例化对象**：会引入 Design 层 parent/child、跨 Design 对象引用和生命周期问题，未采用。
- **每个 Design 复制所有 base Module/库**：完全自包含但内存/加载成本高，未采用；改用共享只读 base + 按需 uniquify。
- **完全可变的跨 Design 共享**：会导致设计之间串扰、不可复现，未采用。

## Log

- 2026-10-19 明确 Design 不维护 helper Module 表；unique Module 仍归原始 Design。
