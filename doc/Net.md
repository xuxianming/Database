# Net

## 角色

`Net` 是 Design/topmodule 级**唯一的逻辑网络**，也是后续物理/寄生数据的主要载体。

- Net 忽略 hierarchy；
- Net 只连接 Pin；
- Net 不连接 HPin；
- Net 不直接连接 ModulePort（ModulePort 通过 io Pin）；
- 一个 Net 可以对应 0..N 个 HNet。

## 关键字段

| 字段 | 含义 |
| :--- | :--- |
| `m_design_id` | 顶层 Design |
| `m_top_module_id` | topmodule |
| `m_hinst_id` | 所属 HInst；顶层 Net 为 invalid |
| `m_hnet_list_id` | 关联 HNet 列表/映射 |
| `m_pin_head_id` | Pin 链表头 |
| `m_next_net_id` / `m_prev_net_id` | Module/Net 链表 |

## 连接语义

Net 只连接 Pin：

```text
Net 1 ── * Pin
```

禁止：

- `Net → HPin`；
- `Net → HNet` 作为物理连接（HNet 是代理关系，不是 Net 的物理端点）；
- `Net → ModulePort` 直接连接。

Pin 可以同时连接 Net 和 HNet：

```text
Net  ── Pin ── HNet
```

## 导航

```cpp
Design* GetDesign() const;   // 顶层 Design
Module* GetModule() const;   // topmodule
HInst*  GetHInst() const;    // 所在 HInst；顶层 Net 为 null
```

## 关键接口

```cpp
void ConnectPin(Pin* pin);
void DisConnectPin(Pin* pin);
```

## 约束

1. Net 在 Design/topmodule 级唯一；
2. Net 只连接 Pin；
3. Net 不连接 HPin；
4. Net 承载 physical/parasitic 数据；
5. 一个 Net 可以有 0..N 个 HNet；
6. Net 不因多个 HNet 而复制。

## 当前实现

已实现：

- `Net` 对象和 Pin 链表；
- `GetDesign`/`GetModule`/`GetHInst`；
- `ConnectPin`/`DisConnectPin`。

待后续：

- Net 创建 API 与 topmodule 归属赋值；
- HNet 列表/映射的完整维护；
- physical/parasitic 字段；
- 与 ModulePort io Pin 的完整连接建立。
