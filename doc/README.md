# Logic DB 类级设计文档

本目录整理当前 **logic DB** 的类级设计，按对象拆分，方便后续扩展 hierarchy、connectivity、physical 等能力。

PDR、ADR、issue 仍在 [`docs/`](../docs/)；本目录只描述当前讨论定稿的 logic 对象模型和实现边界。

## 阅读顺序

1. [00-overview.md](00-overview.md) — logic 设计理念、对象关系和核心不变量
2. 按类阅读：
   - [Design.md](Design.md)
   - [Module.md](Module.md)
   - [HInst.md](HInst.md)
   - [Inst.md](Inst.md)
   - [Net.md](Net.md)
   - [Pin.md](Pin.md)
   - [HNet.md](HNet.md)
   - [HPin.md](HPin.md)
   - [ModulePort.md](ModulePort.md)
   - [Port.md](Port.md)
3. [SlangImport.md](SlangImport.md) — slang RTL 导入、对象映射与分阶段扩展设计
4. [DFG.md](DFG.md) — DFGGraph/DFGPrim/DFGNode/DFGPin/DFGEdge 建模与 slang 吸收

## 范围

本目录覆盖当前 logic 层的核心对象：

`Design`、`Module`、`HInst`、`Inst`、`Net`、`Pin`、`HNet`、`HPin`、`ModulePort`、`Port`。

以下内容暂不在本目录展开：

- `BusPort` / `BusNet`：当前仍是占位/后续能力；
- LEF/TechLib/TimingLib 细节：属于 library/physical 文档；
- 具体磁盘二进制格式：属于 persistence/NFR 文档；
- 迭代器和内存管理器：类文档只描述对象职责，不逐个展开实现容器。

## 文档约定

- **目标模型**与**当前实现状态**分开写；
- “应具备”的行为不自动等于“已实现”；
- 对象关系以 `Design → topmodule → HInst tree` 为入口；
- 导航语义统一为 `GetDesign` / `GetModule` / `GetHInst`；
- 连接语义统一为 `Net ↔ Pin`、`Pin ↔ HNet`、`HNet ↔ Pin/HPin`。

## 关联文档

- [仓库 README](../README.md)
- [根目录 DESIGN.md](../DESIGN.md)
- [PDR 架构](../docs/pdr/01-architecture/00-README.md)
- [对象模型规范](../docs/pdr/01-architecture/01-spec.md)
- [概念接口](../docs/pdr/01-architecture/02-interfaces.md)
- [术语表](../CONTEXT.md)
