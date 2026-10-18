# 统一层次导航接口
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-19 ｜ 关联: ADR-0003 / ADR-0006 / ADR-0007 / ADR-0008

## 目标
把层次导航统一为 `GetDesign` / `GetModule` / `GetHInst`：对象在创建或 hierarchy expansion 时写入导航字段，查询直接返回，不沿 parent 链循环。

## 约定
- `GetDesign`：`Module`/`HInst`/`Inst`/`Net`/`HNet`/`HPin` 返回顶层 Design；`Module` 返回原始 Design，unique 副本保留原 Design。
- `GetModule`：`HInst`/`Inst`/`Net`/`HNet` 返回 topmodule；`HInst` 用 `m_module_id`，其余对象用展开时写入的 topmodule id。
- `GetHInst`：`HInst` 返回上层 HInst，`Inst`/`Net`/`HNet` 返回所在 HInst；顶层 HInst 与 topmodule 直属对象返回 null。
- HInst 管理内部 Inst/HNet/HPin 的链表；未展开时为空。`Inst` 内部保留定义 Module 用于名称/路径/列表，但只公开 `GetModule`（返回 topmodule）。
- 统一使用 `GetDesign`，不保留 `GetMasterDesign` 兼容别名。

## 代码范围
- `Module`：增加 `GetDesign`。
- `HInst`：增加 `GetDesign`、`GetHInst`；增加内部 Inst/HNet 链表头；迭代器改用 HInst 链表。
- `Inst`：增加 `m_top_module_id`、`m_hinst_id`、`GetDesign`、`GetHInst`；`GetModule` 返回 topmodule；内部直接使用定义 Module；`PathName` 优先 HInst。
- `Net`：增加 `m_top_module_id`、`m_hinst_id`、`GetDesign`、`GetModule`、`GetHInst`。
- `HNet`：增加 `m_top_module_id`；`GetModule` 改为 topmodule；`GetDesign` 走顶层；保留 owner 字段与 `GetHInst`。
- `HPin`：增加 `GetDesign`。
- `Design`：创建 HInst/Inst 时写入顶层 Design/topmodule/hinst 字段；去掉未使用的 module 表字段。
- 修正 `DestroyInst` 等内部代码：直接使用 Inst 的定义 Module 字段，不通过公开 `GetModule` 取 owner。

## 完成判据
- 新增 `navigation_api_test`：topmodule 下 Inst 的 `GetModule` 为 topmodule，`GetHInst` 为 null；HInst 的 `GetModule` 为 topmodule、`GetHInst` 为自身；`GetDesign` 正确。
- 现有 `db_ut` / `ctest` 全过；默认 `./build.sh -c` 通过。
- hierarchy expansion 的递归创建与 per-HInst Inst/HNet 克隆仍作为后续 issue，不在本条目内实现。

## 步骤
- [x] 规划（接口/字段/兼容层确认）
- [x] 骨架（头文件与导航字段）
- [x] 细节（创建/展开赋值、内部调用修正）
- [x] 异常（空 topmodule、未展开、销毁顺序）
- [x] 回顾（文档同步与测试）

## 结果
- `Module`/`HInst`/`Inst`/`Net`/`HNet`/`HPin` 增加 `GetDesign`，移除 `GetMasterDesign`。
- `HInst`/`Inst`/`Net`/`HNet` 增加 `GetModule`/`GetHInst` 的顶层语义；`HInst::GetHInst` 返回上层 HInst；移除 `HInst::GetUpperHInst`、`HInst::GetTopModule` 与冗余 `HInstNetIterator`。
- `HInst` 增加 `m_design_id`、内部 Inst/HNet 链表头；`HInstInstIterator`/`HInstHNetIterator` 改为遍历 HInst 自有链表（未展开时为空）。
- `Inst` 增加 `m_design_id`/`m_top_module_id`/`m_hinst_id`；`Net`/`HNet`/`HPin` 增加导航缓存字段。
- `Module::Clone` 保留原始 `m_design_id`；unique Module 仍归原始 Design。
- 新增 `navigation_api_test` 与 `connectivity_test`，`ctest` 39/39；`./build.sh -c` 通过。
- 接口清理：移除 `GetMasterDesign`、`HInst::GetTopModule`、`HInst::GetUpperHInst`、`HInstNetIterator`、`Module::PathName`、`Inst::GetOwnerModule`；`ModulePort::GetMasterDesign` 改为 `GetDesign`。
- 连接模型修正：Net 只连 Pin；Pin 可同时连 Net/HNet；HNet 连 Pin 和 HPin；保留/恢复 `Pin::GetHNet/ConnectHNet/DisConnectHNet`、`HNet::ConnectPin/DisConnectPin`、`ModulePort::GetHNet`。
- 未实现递归 hierarchy expansion 与 per-HInst Inst/HNet 克隆；HInst 内部链表当前保持为空，后续单独处理。

## Log
- 2026-10-19 根据用户修正：unique Module 保留原始 Design；统一 GetDesign/GetModule/GetHInst；实现导航字段与 HInst 内部链表骨架。
- 2026-10-19 测试 38/38、默认构建通过；归档。

- 2026-10-19 修正 `HInst::GetHInst`：返回上层 HInst，不再返回自身；顶层 HInst 返回 null。
- 2026-10-19 移除 `GetMasterDesign`、`HInst::GetTopModule`、`HInst::GetUpperHInst` 与 `HInstNetIterator`，统一使用 `GetDesign`/`GetModule`/`GetHInst`。
- 2026-10-19 接口审计：移除旧别名、HInst 祖先接口、HInstNetIterator、Module::PathName、Inst::GetOwnerModule；保留 Pin↔Net/HNet 与 HNet↔Pin/HPin 连接。
- 2026-10-19 纠正连接模型：Net 只连 Pin；Pin 可连 Net/HNet；HNet 可连 Pin/HPin；测试 39/39。
