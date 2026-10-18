# Port

## 角色

`Port` 是端口对象的基类，当前主要服务于 `ModulePort`。

- 保存方向 `Direction`；
- 不直接参与逻辑连接；
- 具体连接能力由 `ModulePort` 及其 io Pin 提供；
- `BusPort` 属于后续/占位能力，不在本 logic 类文档展开。

## 关系

```text
Object
  └── Port
        └── ModulePort
```

## 约束

1. Port 不是 Net/Pin/HNet 的连接端点；
2. 端口方向由 `Direction` 表达；
3. ModulePort 通过 io Pin 接入连接；
4. BusPort 尚未进入当前 logic 连接模型。

## 当前实现

- `Port` 仅保存 `m_dir`；
- `ModulePort` 继承 Port；
- 更完整的 port 类型/方向/总线处理待后续。
