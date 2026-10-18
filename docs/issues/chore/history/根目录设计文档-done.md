# 根目录 DB 设计文档
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-19 ｜ 关联: PDR 01-architecture / ADR-0002..0008

在仓库根目录新增 Markdown 版 DB 设计文档，汇总此前讨论的 Design/Module/HInst/Inst/Net/Pin/HNet/HPin 模型、统一导航接口、连接规则、持久化边界和当前实现状态。

## 完成判据
- 根目录存在 `DESIGN.md`，内容与 `docs/pdr/01-architecture`、`CONTEXT.md`、ADR 一致。
- 明确区分目标模型与当前实现状态，不把尚未实现的递归展开写成已完成。
- 给出对象关系、导航表、连接表、不变量和参考文档。

## 步骤
- [x] 规划（汇总既有 PDR/ADR/CONTEXT）
- [x] 骨架（根目录 DESIGN.md 结构）
- [x] 细节（对象/导航/连接/持久化）
- [x] 回顾（一致性核对）

## 结果
- 新增根目录 `DESIGN.md`，汇总 Design/Module/HInst/Inst/Net/Pin/HNet/HPin、统一导航、连接规则、持久化边界和实现状态。
- 明确区分目标模型与当前实现：递归 expansion、per-HInst Inst/HNet 克隆、自动 HNet 挂接、uniquify 完整对象复制仍为待办。
- 与 PDR 01-architecture、CONTEXT.md、ADR-0002/0003/0005/0006/0007/0008 对齐。
- 按最新修正：Design 只持有 topmodule，不持有 HInst 链表；topmodule 持有顶层 HInst 列表并递归处理层次。
- 代码同步：Design 移除顶层 HInst 链表/DesignTopHInstIterator；Module 增加顶层 HInst 链表与 ModuleHInstIterator；HInst 子链表保持不变；测试 39/39。

## Log
- 2026-10-19 根据讨论汇总根目录 Markdown 设计文档；归档。
- 2026-10-19 修正：Design 不再持有 HInst 链表，顶层 HInst 列表归属 topmodule。
