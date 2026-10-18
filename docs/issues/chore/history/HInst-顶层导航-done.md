# 移除 Module::GetTopModule，改为 HInst 顶层导航
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-18 ｜ 关联: 架构文档（done）

`Module` 是逻辑定义，不应拥有“当前 Design 的顶层”概念。顶层导航属于层次出现 `HInst`：从任一 HInst 沿父链向上，最终到达 topmodule。`Design` 仍拥有 `top_module` 根。

## 完成判据
- 删除 `Module::GetTopModule` 及 `Module::m_is_top`（不再用 Module 表示 top）。
- `HInst::GetTopModule()` 沿 `GetUpperHInst()` 父链导航；顶层 HInst 返回其父 Module（即 topmodule）。
- `Module::GetViewHInst` 不再依赖 top 标记。
- docs 更新：Module 无 top 概念；HInst 提供 top 导航。
- `db_ut` / ctest 37/37、ASan/UBSan 通过。

## 结果
- 删除 `Module::GetTopModule` 与 `Module::m_is_top`；Module 不再表达顶层概念。
- 新增 `HInst::GetTopModule()`：优先沿 HInst 父链；未 unfold 的共享模板无法遍历父链时回退 Design 的 topmodule。
- `Module::GetViewHInst` 不再依赖 top 标记；`Design::GetTopModule` 保留 Design 根语义。
- `path_name_test` 增加 `HInst::GetTopModule()==top`；db_ut 37/37、ctest 37/37、ASan/UBSan 通过。
- PDR `01-spec`、`02-interfaces`、`design/hierarchy.md` 与 CONTEXT 已同步。

## 步骤
- [x] 删除 Module top 接口/字段
- [x] 增加 HInst::GetTopModule
- [x] 更新 path/name 测试与文档
- [x] 构建/ctest/ASan 验证
- [x] 收尾归档

## Log
- 2026-10-18 根据架构讨论调整顶层导航接口。
- 2026-10-18 实现完成；测试/文档通过；收尾归档。
