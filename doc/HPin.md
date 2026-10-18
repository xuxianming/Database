# HPin

## 角色

`HPin` 是 HInst 的**层次边界引脚**，由 HNet 接入。

- 归 HInst；
- 不提供 `GetModule()`；
- 不直接连接 Net；
- 通过 HNet 参与层次内完整连接。

## 关键字段

| 字段 | 含义 |
| :--- | :--- |
| `m_port_id` | 对应的 ModulePort |
| `m_design_id` | 顶层 Design |
| `m_hinst_id` | 所属 HInst |
| `m_upper_hnet_id` | 上层 HNet |
| `m_lower_hnet_id` | 下层 HNet |
| `m_next_hpin_id` | HInst/HNet 的 HPin 链表 |

## 连接语义

```text
上层 HNet ── HPin ── 下层 HNet
```

- HPin 可以连接 upper/lower HNet；
- HPin 不连接 Net；
- HPin 的 `GetNet()` 通过 HNet 间接取得；
- HPin 是层次边界，不是顶层逻辑网络端点。

## 导航

```cpp
ModulePort* GetModulePort() const;
Design* GetDesign() const;   // 顶层 Design
HInst* GetHInst() const;     // 所属 HInst

HNet* GetUpperHNet() const;
HNet* GetLowerHNet() const;
Net*  GetNet() const;        // 通过 HNet 间接得到
```

不提供：

```cpp
Module* GetModule() const;   // 不提供
```

## 关键接口

```cpp
void ConnectUpperHNet(HNet* hnet);
void ConnectLowerHNet(HNet* hnet);
void DisConnectUpperHNet();
void DisConnectLowerHNet();
```

## 约束

1. HPin 归 HInst；
2. HPin 只通过 HNet 接入；
3. Net 不连接 HPin；
4. HNet 可以连接 HPin；
5. HPin 不提供 GetModule；
6. upper/lower HNet 字段与 HNet 的 HPin 链表双向维护。

## 当前实现

已实现：

- ModulePort 关联；
- upper/lower HNet 字段与连接 API；
- `GetDesign`/`GetHInst`/`GetNet`；
- 不提供 `GetModule`。

待后续：

- hierarchy expansion 自动创建并连接边界 HPin；
- 与 ModulePort io Pin 的完整绑定；
- 物理阶段的边界属性。
