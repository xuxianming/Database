# StringMgr P1：arena + 开放寻址
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-17 ｜ 关联: 改进-StringMgr字符串管理（done）

当前 `StringMgr` 已正确（`0=invalid`、`1=空串`、append-only、去重、稳定 view、`DBST` Save/Load），但内部是 `std::deque<std::string>` + `std::unordered_map<std::string_view, IdType>`：每个唯一名一次堆分配，哈希表节点开销大。EDA 规模下需要更紧凑的存储。

## 方案
- 用 64 KiB block arena 存字符串字节；`std::vector<std::string_view>` 作为 ID 表（index 0 = 空串），view 指向 arena，保持稳定。
- 用开放寻址哈希表 `{uint32 hash, IdType id}`（load factor ≤0.7，capacity 为 2 的幂）替代 `unordered_map`；`Find` 全字节比较。
- 保留 API/语义：`0` invalid、`1` empty、IDs append-only、`AddString` 去重、`GetString` 稳定、`Find`、`Clear`、`Save/Load`（`DBST` v1 格式不变）。
- `StringMgr` 仍不可拷贝/移动；写入单线程，Seal 后只读。

## 完成判据
- 现有 `string_mgr_test` 全部通过；新增 100k+ 唯一名触发 rehash、arena block 边界、长字符串、Save/Load 大表测试。
- `AddString` 重复返回同 ID；`GetString` view 在后续 Add 后有效；`Count/Size/Empty` 语义不变。
- 无 ASan/UBSan 报错；内存统计（`PrintStats`）可用。

## 结果
- 64 KiB arena + `std::vector<std::string_view>` + 开放寻址（FNV-1a 32、load factor ≤0.7、2 的幂容量）替换 deque/unordered_map；API/语义/`DBST` v1 格式不变。
- 120k 唯一名压力测试覆盖多次 rehash、约 20 个 arena block、Save/Load 大表；arena 边界、65536/70000 长串、view 稳定性、截断 payload 均有测试。
- 验证：ctest 6/6、直接运行、ASan/UBSan 通过；`PrintStats` 输出 unique/size/bytes/blocks。
- 限制：native-endian/ABI；非线程安全；`DestroyString` 仍 no-op；所有包含 StringMgr 的测试二进制需在头文件变化后完整重建。

## 步骤
- [x] 规划（确认 arena/hash 布局与 Save/Load 兼容）
- [x] 骨架（arena + 开放寻址最小实现 + 现有测试）
- [x] 细节（rehash、长串、Load 重建、Stats）
- [x] 异常（空串、重复、越界、损坏/截断文件）
- [x] 回顾（收尾回顾：文档同步）

## Log
- 2026-10-17 IdArray 持久化完成后，按顺序进入 StringMgr P1；创建条目。
- 2026-10-17 arena + 开放寻址完成；ctest 6/6、ASan/UBSan 通过；收尾归档。
