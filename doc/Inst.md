# Inst

## 角色

`Inst` 是 Module 内部的**叶实例**，目标是 `LibCell`。

- 不参与层次展开；
- 不会产生 child HInst；
- Module 实例化永远产生 HInst，不会产生可展开的 Inst；
- Inst 是逻辑网表中最底层的实例对象。

## 关键字段

| 字段 | 含义 |
| :--- | :--- |
| `m_module_id` | 定义它的 Module（内部字段） |
| `m_design_id` | 顶层 Design |
| `m_top_module_id` | topmodule |
| `m_hinst_id` | 所在 HInst；topmodule 直属 Inst 为 invalid |
| `m_libcell_id` | 目标 LibCell |
| `m_macro_id` | 物理阶段 LEF Macro |
| `m_pins_array_id` | Pin 数组 |
| `m_next_inst_id` / `m_prev_inst_id` | Module Inst 链表 |

## 定义与 occurrence

Inst 作为 Module 内的叶实例定义，展开后会成为某个 HInst 的内部实例。

- 定义 Module 是 Inst 的 owner；
- **不公开 owner 接口**；owner 仅用于名称/PathName/Module 链表/销毁；
- `GetModule()` 公开返回 topmodule；
- `GetHInst()` 返回所在 HInst；
- topmodule 直属 Inst 的 `GetHInst()` 返回 null。

## 导航

```cpp
Module* GetModule() const;   // topmodule
HInst*  GetHInst() const;    // 所在 HInst；topmodule 直属为 null
Design* GetDesign() const;   // 顶层 Design
```

统一原则：

- `GetModule()` 不返回 owner Module；
- 不再保留 `Inst::GetOwnerModule` 公开接口；
- 需要定义 Module 时由内部字段/Module 关系维护。

## 连接

Inst 通过 Pin 接入网络：

- Pin 可以连接 Net；
- Pin 也可以连接 HNet；
- 同一个 Pin 可以同时有 Net 和 HNet；
- Inst 本身不直接保存 Net/HNet，连接由 Pin 持有。

## PathName

- 若在 HInst 内，前缀使用 HInst 的 PathName；
- 否则使用 owner Module 名；
- 末尾拼 Inst 局部名；
- 不在 Inst 上保存 flat path。

## 关键接口

```cpp
Module* GetModule() const;
HInst* GetHInst() const;
Design* GetDesign() const;

LibCell* GetLibCell() const;
uint32_t PinCount() const;
InstPinsArray GetPinsArray() const;

bool Rename(std::string_view name);
void Connect(LogicPort port, Net* net); // 目标接口，当前占位
```

## 约束

1. Inst 不可展开；
2. Inst 不产生 child HInst；
3. `GetModule()` 返回 topmodule；
4. `GetHInst()` 返回所在 HInst，topmodule 直属为 null；
5. 定义 Module 不通过公开 owner API 暴露；
6. Inst 的连接通过 Pin 表达。

## 当前实现

已实现：

- `m_design_id`/`m_top_module_id`/`m_hinst_id`；
- `GetModule`/`GetHInst`/`GetDesign`；
- LibCell、Pin 数组、PathName；
- 移除公开 `GetOwnerModule`；
- 内部通过 `m_module_id` 访问定义 Module。

待后续：

- 每个 HInst occurrence 的 Inst 克隆与归属；
- Pin 的完整连接赋值；
- 物理阶段的 Macro 绑定。
