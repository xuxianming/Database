# DFG RTL/Logic 建模设计
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-19 ｜ 关联: doc/DFG.md / doc/SlangImport.md

定义 DFG canonical semantic graph：DFGLink（DFGGraph/DFGPrim）、DFGNode、DFGPin、DFGEdge，并说明 graph/prim/node/pin/edge 的 type 分类、与现有 Module/HInst/Inst/Net/Pin 的对应，以及 slang 符号如何吸收。

## 结果
- 新增 `doc/DFG.md`。
- 定义 DFGGraph 可展开、DFGPrim 不可展开的 link 模型。
- 给出 graph type、prim type、operator type、port/pin/edge 建议；node 不再有 type。
- 明确 Module/HInst/Inst/Net/Pin 与 DFG 的对应关系。
- 给出 slang DefinitionSymbol/InstanceSymbol/InstanceBodySymbol/Port/PortConnection/Net/Variable/Assign/Process/Generate/Primitive 等映射。
- 明确 AST 不进入 DB；DFG 保存语义化后的完整 RTL 行为图。

## Log
- 2026-10-19 生成 DFG 建模设计文档并归档。
