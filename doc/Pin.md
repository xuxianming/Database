# Pin

## 角色

`Pin` 是 Inst 或 Module port 的连接点，是 **Net/HNet 的连接桥**。

Pin 不只是一个 Net 端点；它同时保存：

- `m_net_id`：全局 Net 连接；
- `m_hnet_id`：当前 HInst 层级的 HNet 连接。

因此同一个 Pin 可以同时属于 Net 和 HNet。

## 关键字段

| 字段 | 含义 |
| :--- | :--- |
| `m_net_id` | 连接的 Net |
| `m_hnet_id` | 连接的 HNet |
| `m_port_id` | ModulePort/LibPort |
| `m_inst_id` | 所属 Inst |
| `m_next_pin_id` / `m_prev_pin_id` | Net 的 Pin 链表 |
| `m_hnet_next_pin_id` / `m_hnet_prev_pin_id` | HNet 的 Pin 链表 |
| `m_is_iopin` | 是否 Module io Pin |

## 连接语义

```text
Net  ── Pin ── HNet
        │
      Inst/ModulePort
```

允许：

- Pin ↔ Net；
- Pin ↔ HNet；
- 同一个 Pin 同时连接 Net 和 HNet。

禁止/边界：

- Net 不直接连接 HPin；
- HNet 可以连接 Pin 和 HPin；
- Pin 不直接持有 HPin。

## 导航与访问

```cpp
Net* GetNet() const;
HNet* GetHNet() const;

Inst* GetInst() const;
ModulePort* GetModulePort() const;
LibPort* GetLibPort() const;

void ConnectNet(Net* net);
void DisConnectNet();

void ConnectHNet(HNet* hnet);
void DisConnectHNet();
```

说明：

- Pin 不公开 `GetModule()`；需要 topmodule 时通过所属 Inst/ModulePort 获取；
- Pin 的 Design/topmodule 导航由所属对象或连接上下文决定；
- `GetHNet()` 是 HNet 反向查询入口。

## 归属

- Pin 通常由 Inst 创建；
- ModulePort 也有自己的 io Pin；
- Pin 的 Net/HNet 链表由对应网络维护；
- Pin 的 `m_net_id` 和 `m_hnet_id` 是两套独立连接，不互斥。

## 约束

1. Net 只连接 Pin；
2. HNet 可以连接 Pin；
3. Pin 可以同时连接 Net 和 HNet；
4. Pin 的 Net/HNet 连接必须由对应网络的反向链表保持一致；
5. Pin 不作为 HInst 层次节点。

## 当前实现

已实现：

- `m_net_id` + `m_hnet_id`；
- `GetNet`/`GetHNet`；
- `ConnectNet`/`DisConnectNet`；
- `ConnectHNet`/`DisConnectHNet`；
- Net/HNet 两侧 Pin 链表维护。

待后续：

- 自动由 hierarchy expansion 建立 Pin/HNet 连接；
- ModulePort io Pin 与 Net/HNet 的完整绑定；
- 物理阶段的 Pin 物理属性。
