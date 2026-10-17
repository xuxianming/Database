# StringMgr Seal：只读契约
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-17 ｜ 关联: 改进-StringMgr字符串管理（done）

`StringMgr` 目前没有“构建完成后只读”的显式契约；EDA flow 在 elaboration 后多线程读名字，写入应被禁止。增加 `Seal()` / `IsSealed()`，Seal 后 `AddString` 抛 `std::logic_error`，`Find/GetString/Size/Count` 仍可用。`Clear()`/`Load()` 重新打开池。

## 完成判据
- Seal 后 Add 被拒绝，Find/Get 正常；Seal 幂等。
- Clear/Load 解除 Seal；Save 在 Seal 状态下可正常工作。
- 现有测试 + Seal 测试通过；ASan/UBSan 通过。

## 结果
- 增加 `Seal()` / `IsSealed()`；Seal 后 `AddString` 抛 `std::logic_error`，`Find/GetString/Size/Count` 保持可用；Seal 幂等。
- `Clear()` / `Load()` 重新打开池；`Save()` 在 Seal 状态下正常。
- `PrintStats` 增加 sealed 状态。
- 验证：`string_mgr_test` 新增 `TestSeal`；ctest 8/8、ASan/UBSan 通过。
- 说明：这是写入契约，不是锁；多线程读依赖调用方在 Seal 后不再写。

## 步骤
- [x] 规划（Seal 语义：Clear/Load 重开）
- [x] 实现 Seal/IsSealed + Add 检查
- [x] 测试（Seal、Add 抛错、Clear/Load 重开、Save）
- [x] 回顾（收尾回顾：文档同步）

## Log
- 2026-10-17 Liberty manager 完成后，进入 StringMgr 只读契约收尾。
- 2026-10-17 实现完成；ctest 8/8、ASan/UBSan 通过；收尾归档。
