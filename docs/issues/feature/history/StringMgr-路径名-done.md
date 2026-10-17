# StringMgr P1.2：路径名合成
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-17 ｜ 关联: 改进-StringMgr字符串管理（done）

当前 `Object::PathName()` 返回空串；`HInst::m_path_name_id` 声明但从未赋值；层次路径没有实际实现。如果把每条路径都 flat intern，内存是 O(N·depth)，而且同一个 Module/Inst 在不同 HInst view 下路径不同，不能存在扁平对象上。

## 方案
- 路径名按需合成，不新增 flat 字符串池：
  - `Module::PathName()`：若有 view HInst（`m_view_hinst_id`）用其路径，否则用自身 name。
  - `HInst::PathName()`：优先父 HInst 路径，否则父 Module 路径，再拼自身 name；`HInst::GetUpperHInst()` 只在 unfold 后可用。
  - `Inst::PathName()`：所属 Module 路径 + 自身 name。
- 补齐缺失的 `HInst::Create` 定义（`Module::CreateHInst` 依赖；当前链接未解析），设置 parent module/template id。
- `HInst::m_unfold` 显式初始化为 0；`m_path_name_id` 暂保留但不再用于 flat path（后续可删）。
- 不处理 bus/bit、Liberty、并发；留给后续 P1 项。

## 完成判据
- `path_name_test`：top module "top"、HInst "u0" -> `top/u0`；Inst "i0" -> `top/i0`；child module 自身路径正确。
- `HInst::Create` 可链接并创建对象；`HInst::GetName/PathName` 正常。
- ctest / ASan/UBSan 通过；不引入 flat path 存储。

## 结果
- 补齐 `HInst::Create(Module*,Module*)` / `(ObjectId,ObjectId)`，解析库中 `HInst::Create` 未定义符号；`Create(Module*,OperatorType)` 明确返回 nullptr。
- `Module::PathName`（view HInst 优先，否则自身 name）、`HInst::PathName`（父 HInst 优先，否则父 Module + name）、`Inst::PathName`（Module + name）按需合成，不写 flat path。
- `HInst::m_unfold` 初始化为 0；`m_path_name_id` 保留但未使用。
- 验证：`path_name_test` 覆盖 top/child、HInst `top/u0`、Inst `top/i0`、虚函数 dispatch；ctest 7/7、ASan/UBSan 通过；`HInst::Create` 未解析符号消失。
- 限制：嵌套 HInst 路径依赖 unfold/`m_view_hinst_id`；`m_path_name_id` 可后续删除；bus/bit/Liberty 路径未做。

## 步骤
- [x] 规划（确认 parent/view 语义与测试树）
- [x] 骨架（补 HInst::Create + PathName 最小实现）
- [x] 细节（Module/Inst/HInst 路径、边界）
- [x] 异常（空 parent、未 unfold、销毁顺序）
- [x] 回顾（收尾回顾：文档同步）

## Log
- 2026-10-17 StringMgr P1.1 完成后，按顺序创建路径名条目。
- 2026-10-17 实现完成；ctest 7/7、ASan/UBSan 通过；收尾归档。
