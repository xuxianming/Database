# 改进 StringMgr 字符串管理
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-17 ｜ 关联: ADR-0001

当前 `BasePlatfrom/StringMgr.h` 采用「短串内联进 ID + 按长度分块表 + 长串动态表」的字符串驻留方案。需要结合 EDA 网表 / Liberty 对象命名的实际分布，评估其在去重、查询、删除、持久化、并发与内存开销上的表现，形成可落地的改进方案。

## 完成判据
- 梳理当前实现事实：ID 位布局、分配 / 回收路径、容量边界、持久化现状。
- 结合 EDA 数据特征（短名重复度、层次路径名、Liberty 名称、批量构建 / 删除）定位主要瓶颈与风险。
- 给出按优先级排序的改进选项（收益 / 成本 / 风险 / 验证方式），并与用户对齐实施范围。

## 结果（分析完成；P0 已完成）
- 已确认：当前 StringMgr 从未被构造；直接实例化会因 LCMRange 含 0 除零、C++20 `std::abs` 非 constexpr 而编译失败；内联编码的长度/flag/返回 view 均有确定性错误；定长表存在索引 off-by-one；无内容去重；无 Save/Load；对象表原样保存 `m_name_id` 而名字池未持久化。
- 建议：保留 `StringMgr` 名称/API，先替换为最小正确版（`deque<string>` + `unordered_map<string_view, NameId>`，去重、稳定 view、Save/Load），再演进到 arena + 开放寻址；不要继续修 127 张定长表 + LCM 块大小。
- 复核补充：定长表 off-by-one 实测 8 字符按 10 字节处理、127 字符越界；`IsPerfectSize(0)` 与 `block_bits` 下溢还会产生 `BLOCK_BYTES=0` / 移位 UB；建议 `0=invalid`、`1=空串`，ID append-only 不复用，v1 删除 `DestroyString`；持久化按 ID 顺序逻辑 dump，load 先于对象表；`ObjectNameId` 保持 32-bit 且独立于 `_UINT_64_`；池作用域（database-global 或 per-design）需显式定；`Pin` 的名通过 `LibPort/ModulePort` 解析，基类 `m_name_id` 对 Pin 是浪费；加显式实例化测试防止“模板坏了但 .so 仍绿”。
- 已定：先最小版；名字随 `.db` 持久化；`ObjectNameId` 保持 32-bit；ECO 删除、多线程、路径缓存留待 P1。

## P0（已确认）
- 最小 StringMgr：`0=invalid`、`1=空串`、append-only、去重、稳定 view、Save/Load。
- 接线：`CreateDesign` 分配 StringMgr + IdArrayMgr；`CreateModule` 写 name；`GetName` 走 owner design。
- 持久化：名字池先于对象表；补 `module_table` / `hnet_table` / `hpin_table` 漏存；load 后重建池。
- 验证：`string_mgr_test` + `design_name_test` + ASan/UBSan。

## P0 结果
- 已落地：`StringMgr` 最小正确版（去重、稳定 view、`0/1` 语义、append-only、Save/Load）+ `Design`/`Module`/`Inst`/`HInst` 的 `GetName` + `SaveAll/LoadAll` 字符串池接线；metadata 写入修正为 `ObjectId`；`strings.db` / `metadata.bin` 格式探针通过。
- 验证：`string_mgr_test`、`design_name_test`、`object_table_test`、`persistence_roundtrip_test`、`id_array_mgr_test`、`inst_create_test` 均通过（ctest 6/6、plain、ASan/UBSan）；`database` 目标构建通过；`design_name_test` 已移除 `_Exit`，正常退出。
- 已修复阻塞：`ObjectTable` 空闲槽覆写 vptr / 双重析构；`LoadAll` 与正常进程退出不再崩溃，持久化 roundtrip 已端到端验证。见 `../../bug/history/ObjectTable-空闲槽覆写vptr-done.md`。
- 已修复阻塞：`IdArrayMgr` 改为 dynamic-only，`CreateInst` 不再因空指针返回；零 port 的 Inst 已通过集成测试。见 `../../bug/history/IdArrayMgr-构造失败-done.md`。
- 未做（P1）：arena + 开放寻址、路径名合成/结构共享、bus bit 命名、Liberty 共享池、并发 freeze、refcount。

## 步骤
- [x] 规划（完成判据与方案已对齐）
- [x] 骨架（最小 StringMgr + 测试；plain / ASan / ctest 通过）
- [x] 细节（接线 Design / Module / Inst / HInst + SaveAll/LoadAll + IdArrayMgr）
- [x] 异常（Save/Load roundtrip 已通过；边界单测、ASan/UBSan 通过）
- [x] 回顾（ADR-0001 + issue 更新；Current 基线未选定，未更新 Current）

## Log
- 2026-10-17 创建条目，开始现状分析。
- 2026-10-17 完成分析；确认构造/内联/定长表/持久化等阻断问题；oracle 独立复核结论一致并补充 ID 语义/池作用域/Pin 名复用等要点；等待范围决策。
- 2026-10-17 用户确认 P0；落 ADR-0001，开始最小 StringMgr（去重 / 稳定 ID / Save-Load）与测试。
- 2026-10-17 P0 核心落地；`string_mgr_test`/`design_name_test` 通过（ctest 2/2、ASan/UBSan）；发现并登记 IdArrayMgr、ObjectTable 两个预存阻塞。
- 2026-10-17 ObjectTable 修复完成；`object_table_test`/`persistence_roundtrip_test` 加入，ctest 4/4、ASan/UBSan 通过；P0 收尾归档。
- 2026-10-17 IdArrayMgr dynamic-only 修复完成；`id_array_mgr_test`/`inst_create_test` 加入，ctest 6/6。
