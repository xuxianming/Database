# StringMgr refcount / ECO
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-17 ｜ 关联: 改进-StringMgr字符串管理（done）

当前名字是 design-owned：`StringMgr::DestroyString` 是 no-op，对象销毁/重命名不会释放名字，`Find` 永远能查到已经无人引用的名字。需要引入引用计数，让 ECO 删除/重命名至少有正确的逻辑生命周期；arena 字节不回收（ID append-only），但 ref=0 的名字不再可 Find，且可被后续 Add 复活。

## 方案
- `StringMgr` 每个 view 增加 `uint32_t ref`：
  - `AddString` 自动 +1（包括空串 ID 1）；返回已有 ID 同样 +1；new ID ref=1。
  - `ReleaseString(id)`（`DestroyString` 别名）−1；0 后不再被 `Find` 返回（index 保留，便于同 ID 复活）；double release 抛 `runtime_error`。
  - `Find` 只返回 ref>0 的名字；`AddString` 对 ref=0 的旧 ID 复活并 ++Count。
  - `Count` = live unique non-empty；`Size` = views（含 dead）；`GetString` 仍可读 dead ID（ID 稳定）。
- `DBST` 升级 v2：每条记录 = len + bytes + uint32 ref；`Load` 兼容 v1（ref 视为 1）。`Save` 写 v2。
- DataModel：
  - `Module::Rename` / `Inst::Rename` / `HInst::Rename`：Add 新名、Release 旧名、更新 name id，并 invalidate name index。
  - `DestroyInst` / `DestroyHInst` / `DestroyModule`：在销毁前通过 owner design 的 StringMgr Release 名字；`DestroyDesign` 直接关闭池。
  - `LoadAll` 使用 v2 refs；对象 raw-load 不再额外 Add。
- 测试：StringMgr refcount/resurrect/Save-Load refs；ECO rename/destroy/Find 可见性；现有 9 个测试全过。
- 限制：arena 不回收、ID 不复用；Seal 只约束 Add（Release 是 ECO 写路径，应在 Seal 前完成）；Liberty 名字仍不做 refcount。

## 完成判据
- Add 两次同一名字 ref=2；Release 一次仍 Find，Release 到 0 后 Find=null，Get 仍可读；再次 Add 返回同 ID 并恢复 live。
- Rename 后旧名不可 Find、新名可 Find/GetName/PathName 正确；Destroy 后名字释放。
- Save/Load v2 保持 ref；v1 兼容；现有持久化测试通过。
- ctest / ASan/UBSan 通过。

## 结果
- `StringMgr` 每个 view 增加 ref：Add +1、Release/DestroyString −1；ref=0 不再被 Find 返回，但 GetString 仍可读、可被 Add 复活；Count 为 live unique。
- `DBST` v2 每条记录增加 uint32 ref；Load 兼容 v1（ref=1），失败 Clear。
- Module/Inst/HInst 增加 Rename（Add 新名、Release 旧名、name index 失效）；DestroyInst/HInst/Module 释放名字，DestroyDesign 关闭整个池。
- 测试：StringMgr refcount/resurrect/v2/v1；`eco_test` 覆盖 Rename、PathName、Find、销毁释放、Save/Load ref roundtrip；ctest 10/10、ASan/UBSan（含 name/path/liberty/inst/persistence）通过。
- 限制：arena/hash 不回收，dead ID 仍可 GetString；Seal 只阻止 Add，Release 仍允许（ECO 应在 Seal 前完成）；重复 view 且首个 dead 的极端 Find 语义仍有限制；DBST v2 native-endian。

## 步骤
- [x] 规划（ref 语义、v2 格式、Seal/Release 边界）
- [x] 骨架（StringMgr ref + Release + Find/Resurrect）
- [x] 细节（DataModel Rename/Destroy release、name index invalidation）
- [x] 异常（double release、dead name、v1/v2）
- [x] 回顾（收尾回顾：文档同步）

## Log
- 2026-10-17 name→object 索引完成后，用户选择 refcount/ECO；创建条目。
- 2026-10-17 实现完成；ctest 10/10、ASan/UBSan 通过；收尾归档。
