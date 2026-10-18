# ModulePort

## 角色

`ModulePort` 是 Module 对外的逻辑端口，继承自 `Port`。

- 属于某个 Module；
- 通过 `io Pin` 连接到 Net/HNet；
- 不是顶层 Net 的直接端点；
- 不直接持有 HInst 层次关系。

## 关键字段

| 字段 | 含义 |
| :--- | :--- |
| `m_module_id` | 所属 Module |
| `m_upper_hpin_id` | 关联的边界 HPin |
| `m_io_pin_id` | io Pin |
| `m_next_port_id` | Module port 链表 |

## 连接

```text
ModulePort ── io Pin ── Net/HNet
```

说明：

- Net 不直接连接 ModulePort；
- ModulePort `GetNet`/`GetHNet` 通过 io Pin 间接获取；
- 层次边界的 HPin 由 HInst 持有，不是 ModulePort 本身；
- `m_upper_hpin_id` 用于 ModulePort 与边界 HPin 的关联。

## 导航

```cpp
Module* GetModule() const;
Design* GetDesign() const; // 所属 Module 的 Design

Pin*  GetIoPin() const;
Net*  GetNet() const;      // 通过 io Pin
HNet* GetHNet() const;     // 通过 io Pin
```

## 约束

1. ModulePort 属于 Module；
2. Net/HNet 连接通过 io Pin；
3. ModulePort 不是 HInst；
4. 边界 HPin 归 HInst；
5. ModulePort 不提供顶层 `GetModule()`/`GetHInst()` 语义；`GetModule()` 返回所属 Module。

## 当前实现

已实现：

- `GetModule`/`GetDesign`；
- `GetIoPin`；
- `GetNet`/`GetHNet`（经 io Pin）。

待后续：

- ModulePort ↔ HPin 的完整绑定；
- port 方向和 bus/bit 处理；
- 物理阶段 port 属性。
