# Logic DB 类级设计文档整理
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-19 ｜ 关联: DESIGN.md / PDR 01-architecture

将 logic DB 的设计整理为根目录 `doc/` 下的类级 Markdown 文档：一个 overview + 每个核心类一个文档，便于后续扩展和按类维护。

## 结果
- 新增 `doc/README.md` 索引。
- 新增 `doc/00-overview.md` logic 设计理念与对象关系。
- 新增 `Design.md`、`Module.md`、`HInst.md`、`Inst.md`、`Net.md`、`Pin.md`、`HNet.md`、`HPin.md`、`ModulePort.md`、`Port.md`。
- 明确目标模型与当前实现状态，覆盖 HInst 顶层列表归 topmodule、Design 不持有 HInst 链表、Net/Pin/HNet/HPin 连接规则。
- `DESIGN.md` 增加 `doc/` 入口链接。

## Log
- 2026-10-19 完成类级文档拆分；当前只覆盖 logic 核心类，BusPort/BusNet 暂列为后续。
