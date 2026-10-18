# 架构文档：Design / Module / Hierarchy
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-18 ｜ 关联: ADR-0002..0005

将 Design 聚合、Module/Inst/HInst 定义-出现模型、层次展开、库/约束、物理扩展整理为 PDR 架构文档与 ADR，并更新 CONTEXT 术语。

## 结果
- 新增 `docs/pdr/01-architecture/`：00-README、01-spec、02-interfaces、03-nfr、design/hierarchy、design/physical。
- 更新 `docs/pdr/00-README.md` 的最终目标、模块索引、阅读顺序。
- 新增 ADR-0002..0005：Design 完整设计/持久化根、Module/Inst/HInst、库共享/SDC MCMM、物理数据归属。
- 更新 `CONTEXT.md` 术语表与 `docs/adr/README.md` 目录。
- 明确待决：共享库在 Design 快照中是内嵌内容还是版本化外部依赖。

## 步骤
- [x] 对齐 Design/Module/HInst 语义与持久化根
- [x] 写 PDR 架构规格
- [x] 写 ADR 与术语表
- [x] 校验链接/目录

## Log
- 2026-10-18 根据用户澄清落第一版架构文档草稿。
