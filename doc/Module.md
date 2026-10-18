# Module

## 角色

`Module` 是逻辑定义/容器，不是层次节点。

Module 包含：

- Module ports（`ModulePort`）；
- 内部 `Inst` 定义；
- 逻辑名称/ID；
- base/unique 身份。

Module 不持有完整 HInst tree。例外：当 Module 是 Design 的 **topmodule** 时，它持有顶层 HInst 列表，作为 Design 的层次入口。

## 定义与出现

Module 是“定义”，HInst 是“出现”：

- 一个 Module 可以被多个 HInst 实例化；
- Module *不*记录每个 occurrence 的 PathName、物理位置或 occurrence 私有状态；
- HInst 记录 `m_template_id` 指向 Module；
- Module 实例化永远产生 HInst，不会产生可展开的 Inst。

## base / unique

- 默认是跨 Design 共享的只读 base 定义；
- 同一个 Module 可以是 Design A 的 topmodule、Design B 的 helper；
- 基础 hierarchy expansion 不 uniquify；
- 需要设计私有修改时，由 HInst 触发 uniquify 生成 unique 副本；
- unique Module **保留原始 Design 成员，不新建 Design**；
- HInst 负责记录引用 base 还是 unique；
- Design 不维护 helper Module 表。

## 顶层职责

当 Module 是 topmodule 时：

- `m_hinst_head_id` 持有顶层 HInst 链表头；
- `ModuleHInstIterator` 遍历顶层 HInst；
- 每个 HInst 的 `m_module_id` 指向该 topmodule；
- HInst 的 child HInst 由 HInst 自己递归持有；
- Design 只持有 topmodule，不持有 HInst 链表。

`Module` 不提供 `GetModule()` / `GetTopModule()` 导航接口；topmodule 通过 `HInst::GetModule()` 获取。

## 关键接口

```cpp
Design* GetDesign() const;

Inst* CreateInst(LibCell* lib_cell, const std::string& name = "");
Inst* CreateInst(ObjectId lib_cell_id, const std::string& name = "");
Inst* CreateInst(LogicType type, const std::string& name = "");

Module* Clone(const std::string& name = "") const;
bool Rename(std::string_view name);

Inst* FindInst(std::string_view name) const;
```

## 名称与路径

- Module 的名称来自原始 Design 的 StringMgr；
- Module 没有唯一 PathName；
- PathName 只属于 HInst（或 occurrence 视图）；
- `Module::GetDesign()` 返回原始 Design；
- unique 副本不因被其他 Design 的 HInst 引用而改变所属 Design。

## 关系

- `Module 1 → * Inst`（定义）
- `topmodule 1 → * HInst`（顶层 occurrence）
- `HInst * → 1 Module`（`m_template_id`）
- `Module 1 → 1 Design`（原始 Design 成员）

## 当前实现

已实现：

- `GetDesign`、`CreateInst`、`Clone`、`Rename`、`FindInst`；
- topmodule 的 `m_hinst_head_id`；
- `ModuleHInstIterator`；
- Module 的 `m_inst_head_id` 链表；
- unique Clone 保留 `m_design_id`。

待后续：

- `HInst::UniquifyModule` 的完整对象复制；
- unique Module 的 Inst/port/Net 等深拷贝；
- base/unique 版本和名称作用域细节；
- 递归 hierarchy expansion。
