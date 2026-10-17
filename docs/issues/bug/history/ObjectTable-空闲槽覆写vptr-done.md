# ObjectTable 空闲槽覆写 vptr，导致 Clear / 进程退出崩溃
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-17 ｜ 关联: 改进-StringMgr字符串管理

`ObjectTable::FreePush` 把下一个空闲 ID 写到对象存储的前 4 字节；对多态对象而言这是 vptr。`TableBlock` 的隐式析构又会析构所有槽位（包括空闲 / 已销毁槽），于是 `ObjectTable::Clear()`、`LoadAll` 或进程正常退出时通过损坏的 vptr 调用析构，触发段错误。

复现：`DesignMemoryMgr::CreateDesign()` 后正常 `return 0` 即 SIGSEGV；`DesignMemoryMgr::LoadAll` 在 `design_table.Load()` 的 `Clear()` 中崩溃。

## 方案
- `TableBlock` 改为 raw storage（`alignas(TYPE) std::byte[BLOCK_SIZE][sizeof(TYPE)]`），其析构不再隐式析构 `TYPE`。
- `TableBlock` 维护 `live` / `free` 标记；`ObjectTable::Make` 标记 live，`Destroy` 标记 free，`Clear` 只析构 live 槽。
- 空闲链表仍可写在 raw storage 的首字节（不再破坏 vptr）；`Load` 时沿 `m_free` 链重建 free 集，其余已用槽标记 live，并校验 live 数 == 文件中的 count。
- `ObjectTable` 析构走 `Clear()`；`Clear` 先析构 live 对象再释放 block。

## 完成判据
- 空闲链表不再破坏对象 vptr；`Make` / `Destroy` / `Clear` / `Load` 全生命周期无双重析构。
- 正常进程退出不崩溃；`LoadAll` roundtrip 可运行。
- 单测覆盖 Make / Destroy / Clear / Save+Load；ASan/UBSan 通过。

## 结果
- `TableBlock` 改为 raw storage + live/free 标记；`ObjectTable` 负责 placement-new / 析构，析构走 `Clear()` 且只析构 live 槽。
- 空闲链表用 `memcpy` 写入 raw storage，不再破坏 vptr；`Load` 沿 `m_free` 链重建 free 集、标记 live、校验 live 数 == count。
- 验证：`object_table_test`、`persistence_roundtrip_test`、`design_name_test`（已移除 `_Exit`）正常退出；ctest 4/4；ASan/UBSan 无报错。
- 已知边界：每个 block 的 slot 0 保留（容量少 1/block）；旧文件若在 block>0 的 slot 0 有对象，会因 live 计数不一致而加载失败（仓库无既有数据）。raw object bytes 的跨进程 / ABI 可移植性仍未解决。

## 步骤
- [x] 规划（确认 live / dead 跟踪方案与对象存储布局）
- [x] 骨架（raw storage + free-list 修复 + 最小复现测试）
- [x] 细节（live bitmap、Clear / Destroy / Load 重建）
- [x] 异常（双重析构、空表、Load 失败路径）
- [x] 回顾（收尾回顾：文档同步）

## Log
- 2026-10-17 由 StringMgr P0 验证发现；确认正常退出与 `LoadAll` 均会崩溃。
- 2026-10-17 用户确认优先修复；开始 raw storage + live bitmap + 独立空闲链方案。
- 2026-10-17 修复完成；`LoadAll` / 正常退出不再崩溃，持久化 roundtrip 测试通过。
