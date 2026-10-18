# HInst/Module 层次模型重构
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-18 ｜ 关联: 架构文档（done）

当前代码把 `Module` 当成 hierarchy 的一层：Module 持有 hinst list/view/unfold，`HInst::m_module_id` 指向父 Module。这与目标架构不一致。

目标：
- `HInst::m_module_id` 永远指向 **topmodule**；hierarchy 展开时直接设置。
- `HInst::m_template_id` 指向该 HInst 实例化的 Module 定义。
- `HInst` 自身持有 parent/children/sibling；Design 持有顶层 HInst 列表。
- `Module` 只做逻辑定义，不参与 hierarchy：删除 hinst list/view/unfold/CreateHInst/FindHInst/ModuleHInstIterator。
- `HInst::GetTopModule()` 直接返回 m_module_id，不循环；`GetUpperHInst()` 直接读 parent。
- PathName 由 topmodule 名 + HInst 父链合成。

## 完成判据
- Module API 中不再有 hinst/unfold/view 概念。
- HInst 树可由 Design 创建/遍历/销毁；top-level HInst 的 m_module_id == topmodule。
- db_ut/ctest 全过；name_index/path_name 测试改用 HInst tree API。

## 结果
- HInst：`m_module_id` 永远为 topmodule；新增 `m_upper_hinst_id`/`m_child_hinst_head_id`；`m_next_hinst_id` 为 sibling；移除 unfold/path_name_id/shared-template 逻辑；`GetTopModule()` 直接返回 top。
- Design：新增 top HInst 列表、`CreateHInst`、`GetTopHInstHead`、`DesignTopHInstIterator`；移除 `Unfold`。
- Module：移除 hinst list/view/unfold/CreateHInst/FindHInst/ModuleHInstIterator；只保留逻辑定义、Inst/名称/路径（local name）。
- Inst：移除 HInst 链接字段，明确为 Module 内部不可展开的叶实例（目标 LibCell）；Module 实例化只能产生 HInst。
- 测试：path_name/name_index/eco/design 迁移到 Design/HInst tree；db_ut 37/37、ctest 37/37、ASan/UBSan 通过。
- 文档：PDR/ADR/CONTEXT 已同步 HInst.m_module_id=top、Module 不参与层次、Inst 为叶实例。

## 步骤
- [x] HInst 字段/导航重构
- [x] Design 顶层 HInst 列表与 Create/Destroy
- [x] Module 移除 hierarchy 角色
- [x] 测试/文档更新
- [x] 构建/ctest/ASan 验证

## Log
- 2026-10-18 根据架构讨论开始 HInst/Module 层次重构。
- 2026-10-18 实现完成；db_ut/ctest 37/37、ASan/UBSan 通过；收尾归档。
