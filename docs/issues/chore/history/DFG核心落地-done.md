# DFG 核心对象与 Design 挂载
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-19 ｜ 关联: doc/DFG.md / DataModel/DFG

在 `include/DataModel/DFG` 与 `src/DataModel/DFG` 落地 DFG 核心对象：DFGLink/DFGGraph/DFGPrim/DFGNode/DFGPort/DFGPin/DFGEdge/DFGType，并通过 DFGMemoryMgr 管理 ObjectTable、生命周期和 Save/Load。

## 结果
- 新增 DFG 头文件与实现。
- Design 增加 `AttachDFG`/`GetDFG`，只接受 `DFGGraphType::Module`。
- DFGMemoryMgr 管理 graph/prim/node/port/pin/edge 表。
- Design 销毁时递归销毁已挂载 graph tree。
- DFG 表接入 DesignMemoryMgr SaveAll/LoadAll/Clear。
- 新增 `dfg_test`：Module 挂载、Interface 拒绝、node/port/pin/edge、Design 销毁、Save/Load roundtrip。
- `ObjectTable` 增加 `IsLive`，DFG getter 不返回已释放对象。

## Log
- 2026-10-19 落地 DFG 核心对象、内存管理和 Design 挂载；测试 41/41。
- 2026-10-19 补充 DFGPort（内置 io pin），删除 DFGNodeType，操作码由 DFGPrim+DFGOperatorType 表达。
