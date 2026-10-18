# HInst

## 角色

`HInst` 是 Module 实例化的**层次 occurrence**，也是 hierarchy 的核心容器。

HInst 不是 Module 的别名：

- Module 是定义；
- HInst 是某一次实例化；
- 同一个 Module 可以被多个 HInst 出现，各自拥有独立 PathName 和 occurrence 数据。

## 关键字段

| 字段 | 含义 |
| :--- | :--- |
| `m_design_id` | 所属顶层 Design |
| `m_module_id` | 永远指向 topmodule |
| `m_template_id` | 该 occurrence 实例化的 Module 定义 |
| `m_upper_hinst_id` | 上层 HInst |
| `m_child_hinst_head_id` | child HInst 链表头 |
| `m_next_hinst_id` | sibling / 顶层列表 next |
| `m_hpin_head_id` | 边界 HPin 链表 |
| `m_inst_head_id` | 内部 Inst 链表（目标态） |
| `m_hnet_head_id` | 内部 HNet 链表（目标态） |

## 持有内容

每个 HInst 管理：

- child HInst；
- 内部 Inst；
- HNet；
- 边界 HPin；
- 上下层链接。

未展开时内部链表为空。展开后从 topmodule 的顶层 HInst 或某个 HInst 自己开始递归向下遍历 child HInst。

## 导航语义

```cpp
Design* GetDesign() const;       // 顶层 Design
Module* GetModule() const;       // topmodule
HInst*  GetHInst() const;        // 上层 HInst；顶层 null
Module* GetTemplateModule() const; // template Module
```

注意：

- `GetHInst()` **不是自身**，而是上层 HInst；
- `GetModule()` 返回 topmodule，不返回 template Module；
- `GetTemplateModule()` 才返回 `m_template_id` 的 Module；
- `m_module_id` 是 topmodule 的直接缓存，不沿 parent 链循环。

## PathName

- PathName 从 topmodule 名开始沿 HInst 父链合成；
- 顶层 HInst 无上层，路径为 `top/<name>`；
- 子 HInst 为 `top/u0/u1`；
- 不在 Module 上保存 flat path。

## 顶层列表

- Design 不持有 HInst 链表；
- topmodule 持有顶层 HInst 列表；
- `HInst::m_next_hinst_id` 同时用于 sibling/顶层链表 next；
- `HInst::m_upper_hinst_id == kInvalidId` 表示顶层 occurrence。

## 迭代器

- `ModuleHInstIterator`：遍历 topmodule 的顶层 HInst；
- `HInstHInstIterator`：遍历当前 HInst 的 child HInst；
- `HInstInstIterator`：遍历当前 HInst 内部 Inst（目标态，当前链表为空）；
- `HInstHNetIterator`：遍历当前 HInst 内部 HNet（目标态，当前链表为空）；
- `HInstHPinIterator`：遍历当前 HInst 的边界 HPin。

已移除：

- `HInst::GetUpperHInst` → 使用 `GetHInst()`；
- `HInst::GetTopModule` → 使用 `GetModule()`；
- `HInstNetIterator`：Net 是顶层全局对象，HInst 不管理 Net 链表。

## 约束

1. `m_module_id` 永远指向 topmodule；
2. `GetHInst()` 返回上层，顶层 null；
3. Module 实例化永远产生 HInst；
4. HInst 是 occurrence 数据容器，Module 定义不写 occurrence 私有状态；
5. 未展开的 HInst 内部链表为空；
6. HInst 的 child HInst 由 HInst 自己持有，顶层 HInst 由 topmodule 持有。

## 当前实现

已实现：

- HInst 树、parent/child/sibling；
- `m_design_id`/`m_module_id`/`m_template_id`；
- `GetDesign`/`GetModule`/`GetHInst`/`GetTemplateModule`；
- PathName；
- topmodule 顶层 HInst 列表；
- HInst 内部 Inst/HNet/HPin 链表字段和迭代器骨架。

待后续：

- 递归 hierarchy expansion；
- 每个 occurrence 的 Inst/HNet 克隆；
- HNet 自动连接该层所有 Pin/HPin；
- 完整 unfold/flatten 语义。
