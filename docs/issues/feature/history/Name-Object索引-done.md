# 名字到对象索引（Module）
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-17 ｜ 关联: StringMgr P1

当前按名字查找 `Inst`/`HInst` 只能遍历链表（而且 `CreateInst/CreateHInst` 没有真正把对象挂进 Module 链表，`ModuleInstIterator` 实际为空）。需要：
- 修复 Module 的 inst/hinst 链表插入与删除；
- 为每个 Module 建立 `ObjectNameId -> ObjectId` 的侧索引，支持 `FindInst(name)` / `FindHInst(name)`；
- 索引不进入 Module 的 raw 布局，避免破坏 ObjectTable 持久化；LoadAll/Destroy 时清理，首次查询按链表懒构建。

## 方案
- Module 增加 `FindInst(std::string_view)` / `FindHInst(std::string_view)`；通过 design StringMgr 的 `Find` 得到 NameId，再查侧索引。
- 侧索引放在 Module.cpp 的静态表（`module id -> {built, inst map, hinst map}`）；不修改 Module 字段/raw 序列化。
- `CreateInst`/`CreateHInst` 插入 Module 链表并 invalidate 该 module 索引；`DestroyInst`/`DestroyHInst` 先 unlink 再 invalidate；`DestroyModule`/`DestroyDesign`/`LoadAll` 清理。
- 首次 Find 时从链表懒构建索引；Module 链表修复后，LoadAll 的 raw 链表也能重建。
- 不改 StringMgr/IdArray/ObjectTable 的格式；不做跨 design 全局索引。

## 完成判据
- 两个 Module 可有同名 Inst/HInst，`FindInst/FindHInst` 各自只返回本 Module 的对象。
- `ModuleInstIterator`/`ModuleHInstIterator` 在 Create 后能遍历到对象。
- Destroy 后 Find 返回 null；LoadAll 后首次 Find 能从保存的链表重建。
- 测试覆盖同名跨 module、缺失名、销毁、Save/Load；ctest / ASan/UBSan 通过。

## 结果
- 修复 Module 的 inst/hinst 链表插入与删除（Create/Unlink），`ModuleInstIterator`/`ModuleHInstIterator` 现在可用。
- Module 增加 `FindInst`/`FindHInst`：通过 StringMgr `Find` 得到 NameId，再查 per-module 侧索引；索引懒构建、不进入 Module raw 布局。
- Create/Destroy/LoadAll 的索引失效与懒重建已接通；Save/Load 后可从 raw-restored 链表重建。
- 测试 `name_index_test` 覆盖同 module/跨 module 同名、缺失名、迭代器、销毁后查找、Save/Load 重建；ctest 9/9、ASan/UBSan 通过。
- 限制：索引不持久化（懒重建）；同名取链表头（最近创建）；静态索引非线程安全；Destroy 顺序要求 module 在 inst/hinst 之后销毁；HInst unlink O(n)。

## 步骤
- [x] 规划（链表插入/删除与侧索引生命周期）
- [x] 骨架（链表 + 懒索引 + Find API）
- [x] 细节（Create/Destroy/Load invalidation、同名）
- [x] 异常（缺失名、销毁后查找、空 module）
- [x] 回顾（收尾回顾：文档同步）

## Log
- 2026-10-17 StringMgr 批次完成后，用户选择 name→object 索引；创建条目。
- 2026-10-17 实现完成；ctest 9/9、ASan/UBSan 通过；收尾归档。
