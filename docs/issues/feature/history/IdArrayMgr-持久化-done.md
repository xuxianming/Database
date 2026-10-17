# IdArrayMgr 持久化
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-17 ｜ 关联: IdArrayMgr-构造失败（done）

`Design::m_idarray_mgr` 目前只存在于内存：`SaveAll/LoadAll` 不保存数组内容，`LoadAll` 安装空 manager，导致加载后的 `Inst::m_pins_array_id` 失效（`PinCount()/GetPinsArray()` 可能抛 `std::out_of_range`）。需要为 `IdArrayMgr` 增加稳定 ID 的 Save/Load，并接入 Design 快照。

## 方案
- `IdArrayMgr` / `DynLenArrayTable` 增加 `Save(std::ostream&)` / `Load(std::istream&)`。
- 级别是 **slot**，不是只存 live 数组：保存 `slot_count` 和每个 ID 的 `{live, len, values}`；free chunk 也保存长度，Load 时按原 ID 顺序重建，保证 `m_pins_array_id` 不变。
- 文件格式：magic `"IDAR"`、version、`sizeof(IdType)`、`sizeof(ValueType)`、`slot_count`，随后每个 slot 的 live flag + len + payload（仅 live）。
- `DesignMemoryMgr::SaveAll/LoadAll` 保存/加载 `id_arrays.db`，顺序在对象表之前/之后对应；缺失或损坏时记错并保持空 manager。
- 范围：`DesignIdArrayMgr`；`LibertyIdArrayMgr` 仍保持独立（尚未初始化/持久化）。

## 完成判据
- `IdArrayMgr` Save/Load roundtrip：ID 稳定、live 值、free 状态与长度、后续 free 复用均正确。
- Design 快照包含 `id_arrays.db`，LoadAll 后 `GetIdArrayMgr()` 非空且可 `Get` 已保存的 ID。
- 测试覆盖空 manager、多长度、free chunk、损坏/截断文件；ctest / ASan 通过。

## 结果
- `IdArrayMgr`/`DynLenArrayTable` 增加 slot 级 Save/Load：`IDAR` magic、version、ID/Value 宽度、slot_count；每个 ID 保存 `{live, len, payload}`；free chunk 也保存长度。
- Load 按原 ID 顺序重建，free chunk 进入 `free_by_len_`；未来 Allocate 可复用/切分，ID 不漂移。
- `DesignMemoryMgr::SaveAll/LoadAll` 增加 `id_arrays.db`，在对象表之前保存/加载；LoadAll 缺失/损坏时保持空 manager 并置 `success=false`。
- 验证：`id_array_mgr_test` 覆盖空 manager、多长度、free、split、二次 roundtrip、坏 magic/version/width/截断/零 slot；`persistence_roundtrip_test` 断言 manager 非空；ctest 6/6、ASan/UBSan 通过。
- 额外探针：Design 上分配 4 元素数组 → SaveAll → LoadAll → `Get(id)` 读回 `10,11,12,13`，`id_arrays.db` 45 字节。
- 限制：LibertyIdArrayMgr 仍未初始化/持久化；native-endian/ABI；无 `id_arrays.db` 的旧快照 LoadAll 返回 false；非当前 Design 的 manager 不处理。

## 步骤
- [x] 规划（确认 slot 级格式与 Design 接线）
- [x] 骨架（Save/Load + 最小 roundtrip 测试）
- [x] 细节（Design SaveAll/LoadAll、free/复用）
- [x] 异常（空表、截断、版本/宽度不匹配）
- [x] 回顾（收尾回顾：文档同步）

## Log
- 2026-10-17 用户按顺序要求先做 IdArray 持久化；创建条目。
- 2026-10-17 实现完成；ctest 6/6、ASan/UBSan 通过；Design 数组 roundtrip 探针通过；收尾归档。
