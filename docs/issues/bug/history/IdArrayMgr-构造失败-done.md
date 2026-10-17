# IdArrayMgr 无法实例化，Inst 创建被阻塞
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-17 ｜ 关联: 改进-StringMgr字符串管理

`DesignIdArrayMgr = IdArrayMgr<ObjectId, ObjectId, 15>` 构造时，`FixLenArrayTable<Len=8..15>` 触发 `static_assert(SLOTS_PER_BLOCK <= (1u << OFFSET_BITS))`（64 KiB block / 8-bit offset）。另外，`DynLenArrayTable::Allocate` 对 `unordered_map::upper_bound` 的使用、以及 `Get` / `Free` 的 ID 解码与表的编码不一致。

结果：`Design::m_idarray_mgr` 无法分配，`Module::CreateInst` 只能返回 `nullptr`；`Inst` / `Pin` 相关路径无法验证。P0 暂不分配该管理器。

## 方案
- 移除 `FixLenArrayTable`（64 KiB block / 8-bit offset 的前提不成立）；`IdArrayMgr` 统一委托给 `DynLenArrayTable`，ID 就是 `data_` 索引，不再用高位猜长度。
- 修 `DynLenArrayTable`：`unordered_map` 换 `std::map`（`upper_bound` 合法）；`BLOCK_ELEMENTS = MaxSize + 1`（长度字占 1 个槽）；增加 free 标记、越界/重复释放检查；`Allocate(0)` 返回空 view。
- `Design` 分配 / 释放 `IdArrayMgr`；`LoadAll` 分配空 manager（数组内容持久化另议）。
- 测试：`id_array_mgr_test` 覆盖 fixed/dynamic lengths、复用、split、double free、越界、Clear；`inst_create_test` 用零 port 的 `LibCell` 验证 `Module::CreateInst` 不再被空指针阻塞。

## 完成判据
- `DesignIdArrayMgr` 可构造；`Allocate` / `Get` / `Free` 的 ID 编解码一致。
- `Inst::Create` 可创建 pin 数组；补单测（固定长度 / 动态长度 / 复用 / 越界）。
- `Design` 分配 IdArrayMgr；`CreateInst` 不再因空指针返回。

## 结果
- `IdArrayMgr` 改为 dynamic-only：删除 `FixLenArrayTable` 与高位长度 tag 分发；ID 是 `data_` 索引（0 保留为空/无效），`FixLen` 仅作为 split slack 保留。
- `DynLenArrayTable` 修 `std::map` + `upper_bound`、`BLOCK_ELEMENTS=MaxSize+1`、长度字 `memcpy`、free 标记、越界/重复释放检查；`Allocate(0)` 返回空 view。
- `Design` 创建/销毁时分配/释放 `IdArrayMgr`；`LoadAll` 重建空管理器；`LibCell::m_ports` 默认 `kInvalidId`，零 port 的 `CreateInst` 可走通。
- 验证：`id_array_mgr_test`、`inst_create_test` 通过；ctest 6/6；两个新测试 ASan/UBSan 无报错。
- 已知限制：数组内容不随 `.db` 持久化；LoadAll 后旧 `m_pins_array_id` 可能失效；Liberty 自身的 array/string manager 仍未初始化；带 port 的 Inst 需要后续 Liberty 配置。

## 步骤
- [x] 规划（核对 block / offset 位宽与 ID 布局）
- [x] 骨架（dynamic-only + Allocate/Get/Free 往返测试）
- [x] 细节（修静态断言、动态表、解码、Design 接线）
- [x] 异常（复用、split、越界、double free、空表）
- [x] 回顾（收尾回顾：文档同步）

## Log
- 2026-10-17 由 StringMgr P0 接线发现；P0 暂不分配 IdArrayMgr。
- 2026-10-17 ObjectTable 已修复，持久化 roundtrip 通过；IdArrayMgr 仍是 Inst 创建的唯一阻塞。
- 2026-10-17 用户要求修复；采用 dynamic-only + ID 索引方案，开始实现与测试。
- 2026-10-17 修复完成；`id_array_mgr_test`、`inst_create_test` 通过，ctest 6/6，ASan/UBSan 通过。
