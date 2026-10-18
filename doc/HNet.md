# HNet

## 角色

`HNet` 是 `Net` 在某个 **HInst 层级**的代理，用于表达该层次内的完整连接。

- HNet 唯一对应一个 Net；
- 一个 Net 可以对应 0..N 个 HNet；
- 每个 Net 每穿过一个 HInst，创建恰好一条 HNet；
- HNet 在所属 HInst 层级连接该层所有 Pin 和 HPin；
- HNet 只承载逻辑层次信息，不承载 physical/parasitic。

## owner

HNet 的 owner 是 **Module xor HInst**：

| owner | 含义 |
| :--- | :--- |
| Module | 顶层 HNet |
| HInst | 展开产生的中间 HNet |

不能同时拥有 Module 和 HInst。

## 关键字段

| 字段 | 含义 |
| :--- | :--- |
| `m_module_id` | owner Module（可为空） |
| `m_hinst_id` | owner HInst（可为空） |
| `m_design_id` | 顶层 Design |
| `m_top_module_id` | topmodule |
| `m_net_id` | 对应 Net |
| `m_pin_head_id` | 本层 Pin 链表 |
| `m_hpin_head_id` | 本层 HPin 链表 |
| `m_next_hnet_id` | HInst/Module HNet 链表 |

## 连接语义

HNet 可以连接：

- Pin；
- HPin。

HNet 不直接承载物理数据；它只解释 Net 在某个层次内的连接视图。

```text
        Net
         │
        HNet
       /    \
    Pin      HPin
```

## 导航

```cpp
Net*    GetNet() const;
HInst*  GetHInst() const;   // 所属 HInst；顶层 HNet 为 null
Module* GetModule() const;  // topmodule
Design* GetDesign() const;  // 顶层 Design
```

注意：

- `GetModule()` 返回 topmodule，不返回 owner/template Module；
- `GetHInst()` 返回 HNet 所属 HInst；
- 顶层 HNet 的 `GetHInst()` 为 null；
- `HPin` 不提供 `GetModule()`。

## 关键接口

```cpp
void ConnectPin(Pin* pin);
void DisConnectPin(Pin* pin);

void ConnectHPin(HPin* hpin);
void DisConnectHPin(HPin* hpin);

void AttachToNet(Net* net);
void AttachToNet(ObjectId net_id);
void DetachFromNet();
```

## 连接一致性

- HNet 的 Pin 链表与 Pin 的 `m_hnet_id` 双向维护；
- HNet 的 HPin 链表与 HPin 的 upper/lower HNet 双向维护；
- 一个 Pin 可以同时连接 Net 和 HNet；
- HNet 不替代 Net 的物理身份。

## 当前实现

已实现：

- owner Module/HInst 字段；
- `m_pin_head_id` / `m_hpin_head_id`；
- Pin/HPin 连接 API；
- `GetNet`/`GetHInst`/`GetModule`/`GetDesign`。

待后续：

- hierarchy expansion 自动创建 HNet；
- 自动连接该层所有 Pin/HPin；
- Net 与 HNet 的一对多映射完整维护；
- topmodule/owner 字段在展开时自动赋值。
