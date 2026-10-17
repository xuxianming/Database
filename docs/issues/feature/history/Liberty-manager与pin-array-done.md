# Liberty manager 初始化与 pin array
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-17 ｜ 关联: StringMgr P1 / IdArrayMgr 持久化

`LibertyMemoryMgr` 的 `m_string_mgr` / `m_idarray_mgr` 一直是 nullptr；`LibCell::GetPortArray()` 因此只能返回空 view，`Module::CreateInst` 目前只验证了零 port cell。需要初始化 Liberty 的 string/array manager，并让带 port 的 `LibCell` 能创建带 pin array 的 `Inst`。

## 方案
- `LibertyMemoryMgr::EnsureManagers()`：按需分配 `LibertyStringMgr` / `LibertyIdArrayMgr`；`CreateLibertyLib/CreateLibCell/CreateLibPort` 调用。
- `LibertyMemoryMgr::Clear()`：释放两个 manager 并清空 Liberty 对象表（当前为空实现）。
- `LibCell` 增加 `SetPorts(LibPortsArrayId, uint32_t count)`，供创建/解析路径设置 port 数组；`GetPortArray` 走已初始化的 LibertyIdArrayMgr。
- 测试 `liberty_ports_test`：创建 2 个 LibPort，构造 LibertyIdArrayMgr 数组并设置到 LibCell；`LibCellPortIterator` 能遍历 2 个 port；Design/Module 创建 `Inst` 后 `PinCount()==2`、`GetPinsArray().Size()==2`；清理 manager 与对象。
- 范围：Liberty manager 初始化 + 带 port pin array；不做 Liberty 持久化、名字解析、bus/bit。

## 完成判据
- `GetStringMgr()/GetIdArrayMgr()` 非空；带 2 port 的 `CreateInst` 产生 2 个 Pin。
- `LibCellPortIterator` 与 `Inst::GetPinsArray()` 正确。
- ctest / ASan/UBSan 通过；`LibertyMemoryMgr::Clear()` 不泄漏。

## 结果
- `LibertyMemoryMgr::EnsureManagers()` 按需分配 `LibertyStringMgr`/`LibertyIdArrayMgr`；`CreateLibertyLib/Cell/Port` 调用；`Clear()` 释放 manager 并清空 Liberty 表，可重复调用。
- `LibCell::SetPorts` 接入 port 数组；2-port 测试覆盖 `LibCellPortIterator`、`Inst::PinCount()==2`、`GetPinsArray()` 与 `Pin` 解析。
- 测试程序显式 `LibertyMemoryMgr::Clear()`；`inst_create_test`/`path_name_test` 也补了清理，避免静态 manager 泄漏。
- 验证：ctest 8/8；`liberty_ports_test`、`inst_create_test`、`path_name_test` ASan/UBSan 通过。
- 限制：Liberty 持久化仍是 stub；Liberty 名字/string pool 尚未被 intern；`SetPorts` 不校验数组；Pin 不归 Inst 所有。

## 步骤
- [x] 规划（确认 manager 生命周期与 port 数组所有权）
- [x] 骨架（EnsureManagers/Clear + LibCell::SetPorts）
- [x] 细节（2-port Inst、迭代器、清理）
- [x] 异常（空 port、重复 Clear、销毁顺序）
- [x] 回顾（收尾回顾：文档同步）

## Log
- 2026-10-17 StringMgr 路径名完成后，按顺序进入 Liberty manager 初始化。
- 2026-10-17 实现完成；ctest 8/8、ASan/UBSan 通过；收尾归档。
