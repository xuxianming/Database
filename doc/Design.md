# Design

## 角色

`Design` 是一份完整用户设计的设计上下文，也是持久化单元。

Design 不是“Design 的实例”，也不能嵌套。它定义：

- 当前设计的 topmodule；
- 对象名称池和对象表边界；
- SDC/MCMM 等设计约束；
- 库引用；
- 展开/打平后的物理视图；
- 持久化快照边界。

## 拥有的内容

- `top_module`：本 Design 的顶层 Module，也是层次入口；
- `StringMgr` / `IdArrayMgr`：名称和动态数组存储；
- 逻辑对象：Module、HInst、Inst、Net、Pin、ModulePort 等；
- SDC/MCMM；
- TechLib/TimingLib 引用；
- 物理视图。

Design **不拥有/不维护**：

- Design 实例或 Design 嵌套；
- helper Module 表；
- HInst 链表；顶层 HInst 列表由 topmodule 持有；
- 跨 Design 的可变对象共享；只读 base Module/库可共享。

## 关键接口

```cpp
Module* GetTopModule() const;
void SetTopModule(ObjectId top_module_id);

Module* CreateModule(const std::string& name = "");
Module* CreateModule(OperatorType type, const std::string& name = "");

HInst* CreateHInst(HInst* parent, Module* template_module, std::string_view name);

DesignStringMgr* GetStringMgr();
DesignIdArrayMgr* GetIdArrayMgr();
```

`CreateHInst` 是创建入口/工厂：

- `parent == nullptr`：把 HInst 链接到 **topmodule 的顶层 HInst 列表**；
- `parent != nullptr`：把 HInst 链接到 parent 的 child HInst 列表。

Design 本身不保存 HInst 链表头；topmodule 的 `m_hinst_head_id` 才是顶层入口。

## current_design

`current_design` 是工具当前活动 Design：

- 大多数查询/修改以它为作用域；
- 多 Design 可并存；
- 工具通常只展开/优化 current_design；
- `load` 的结果是一个完整 Design，不是 Design 的实例。

## 持久化

- 保存/加载以 Design 为单元；
- 快照覆盖 topmodule、从 topmodule 可达的 HInst 树、可达 Modules、名称、约束、物理视图和库解析信息；
- 不要求独立 helper Module 表；
- unique Module 通过 HInst 树可达并随快照保存；
- 具体二进制格式和版本迁移不在本类文档展开。

## 关系

- `Design 1 → 1 topmodule`
- `topmodule 1 → * 顶层 HInst`
- `HInst 1 → * child HInst`
- `Module 1 → * Inst`
- `HInst 1 → * Inst/HNet/HPin`

## 约束

1. Design 不存在实例/嵌套；
2. Design 只持有 topmodule，不持有 HInst 链表；
3. topmodule 是层次入口，顶层 HInst 列表归 topmodule；
4. 从 topmodule 沿 HInst 树可访问全部层；
5. Design 不维护 helper Module 表；
6. 只读 base Module/库可跨 Design 共享，可变对象不可跨 Design 共享。

## 当前实现

已实现：

- `m_top_module_id`；
- `GetTopModule` / `SetTopModule`；
- `CreateModule`；
- `CreateHInst` 工厂；
- Design 私有的 StringMgr/IdArrayMgr；
- Design 无 HInst 链表；topmodule 的 `m_hinst_head_id` 持有顶层 HInst；
- `current_design` 静态切换。

待后续：

- 更完整的 Design 快照模块；
- 多 Design 隔离和 load 完整性校验；
- 物理视图/约束对象的完整实现。
