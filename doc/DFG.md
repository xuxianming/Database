# DFG RTL/Logic 建模

> 状态：候选 canonical semantic graph 设计
>
> 目标：用统一的 `DFGLink / DFGNode / DFGPin / DFGEdge` 表达 slang 导入后的 RTL 定义、实例、连接和行为；AST 不进入 DB。

## 1. 一句话模型

```text
DFGLink = 可被引用的定义
  ├─ DFGGraph = 可继续展开的定义图（module/interface/process/...）
  └─ DFGPrim  = 不可展开的叶子定义（gate/cell/...）

DFGNode = 某个定义的一次出现/一个行为节点
DFGPin  = DFGNode 上的连接点
DFGEdge = DFGPin 之间的连接
```

对应关系：

```text
Module         ≈ DFGGraph(type=Module)
HInst          ≈ DFGNode(link=DFGGraph(Module))
Inst           ≈ DFGNode(link=DFGPrim(Cell/Gate))
Net/HNet       ≈ DFGNode(link=DFGGraph(Signal)) + DFGEdge
Pin/HPin       ≈ DFGPin
ModulePort     ≈ DFGPort + io DFGPin
```

## 2. 类关系

```cpp
class DFGLink {
    DFGLinkKind linkKind;   // Graph | Prim
    ObjectNameId name;
    SourceId source;
    // parameter interface, attributes...
};

class DFGGraph : public DFGLink {
    DFGGraphType graphType;
    ObjectId portHead;       // DFGPort list
    ObjectId nodeHead;       // DFGNode list
    ObjectId edgeHead;       // DFGEdge list
    bool expandable() const { return true; }
};

class DFGPrim : public DFGLink {
    DFGPrimType primType;
    DFGOperatorType operatorType; // meaningful when primType == Operator
    bool expandable() const { return false; }
};

struct DFGType {
    uint32_t width;
    bool isSigned;
    bool isFourState;
    // packed/unpacked dimensions, TypeId...
};

class DFGPort : public Object {
    ObjectId ownerGraph;     // Graph on which the port is declared
    ObjectId ioPin;          // built-in io pin
    DFGPinDirection direction;
    DFGType type;
};

class DFGNode {
    ObjectId linkId;         // DFGGraph or DFGPrim
    DFGLinkKind linkKind;    // identifies which table linkId belongs to
    ObjectId parentGraph;
    ObjectId pinHead;
    SourceId source;
    // NO node type: type comes from link graph/prim
};

class DFGPin {
    ObjectId ownerNode;
    ObjectId ownerPort;      // set for a DFGPort io pin
    DFGPinDirection direction;
    DFGType type;
    ObjectId edgeHead;
    ObjectId nextPin;
};

class DFGEdge {
    ObjectId srcPin;
    ObjectId dstPin;
    DFGType type;
    DFGEdgeKind kind;
    SourceId source;
};
```

说明：

- `DFGNode` **没有 node type**；类型来自 `linkId` 指向的 `DFGGraph`/`DFGPrim`；
- `DFGPort` 挂在 `DFGGraph` 上，类似 ModulePort；port 内部有 io pin；
- `DFGPin` 挂在 node 上，是 edge 的端点；
- `DFGEdge` 连接 node pin（或 port io pin）；
- width/signed 放在 `DFGType`；
- `DFGGraph` 可展开，`DFGPrim` 不可展开；
- Graph/Prim 的 type 决定 node/port 的语义。

## 3. DFGLink type

### 3.1 DFGGraphType

建议先覆盖 RTL/逻辑核心：

| 类型 | 含义 | 可展开 |
| :--- | :--- | :--- |
| `Module` | SystemVerilog module | 是 |
| `Interface` | interface | 是 |
| `Program` | program | 是 |
| `Package` | package/namespace | 有限；只保存声明/类型 |
| `Function` / `Task` | 子程序定义 | 不可作为硬件展开；引用/行为 |
| `GenerateBlock` | generate 作用域图 | 是 |
| `Process` | always/initial 行为图 | 是 |
| `Assign` | continuous assign 行为图 | 是 |
| `Control` | if/case/loop 控制流图 | 是 |
| `Signal` | wire/reg/logic 引用/值图 | 是（通常无 body） |
| `Blackbox` | 未解析/黑盒 module | 不可展开或按策略 |
| `Top` | top 图（可选） | 是 |

说明：

- `Module` 是最重要的 Graph；
- `Interface` 需要独立的 port/modport 语义；
- `Package`/`Function` 不是硬件实例，但可以作为 DFGLink 引用；
- `GenerateBlock` 是 scope graph，不是 instance；
- `Process`/`Assign` 是可展开的行为图，不一定对应硬件 module。

### 3.2 DFGPrimType

| 类型 | 示例 |
| :--- | :--- |
| `Gate` | AND/OR/NOT/NAND/NOR/XOR/XNOR/BUF |
| `Mux` | 2:1/4:1 mux |
| `Register` | DFF/DFFR/DFFS |
| `Latch` | latch |
| `Memory` | RAM/ROM/register file |
| `Cell` | LibCell/standard cell |
| `Macro` | hard macro |
| `Const` | 0/1/X/Z/tie |
| `Operator` | 加减乘除、比较、位运算等，具体看 `DFGOperatorType` |
| `Udp` | user-defined primitive |
| `Blackbox` | unresolved leaf |

`DFGPrim` 是不可展开的叶子。与或非、DFF、LibCell、黑盒都属于 `DFGPrim`，只是 `primType` 不同。

`DFGPrimType::Operator` 不笼统：`DFGPrim` 还带 `DFGOperatorType`，完整区分 slang 的 unary/binary operator 和表达式算子。

### 3.3 DFGOperatorType

按 slang 的 `UnaryOperator` / `BinaryOperator` / `ExpressionKind` 建模：

```cpp
enum class DFGOperatorType : uint8_t {
    Unknown = 0,
    // UnaryOperator
    Plus, Minus, BitwiseNot, BitwiseAnd, BitwiseOr, BitwiseXor,
    BitwiseNand, BitwiseNor, BitwiseXnor, LogicalNot,
    Preincrement, Predecrement, Postincrement, Postdecrement,
    // BinaryOperator
    Add, Subtract, Multiply, Divide, Mod, Power,
    BinaryAnd, BinaryOr, BinaryXor, BinaryXnor,
    Equality, Inequality, CaseEquality, CaseInequality,
    WildcardEquality, WildcardInequality,
    GreaterThanEqual, GreaterThan, LessThanEqual, LessThan,
    LogicalAnd, LogicalOr, LogicalImplication, LogicalEquivalence,
    LogicalShiftLeft, LogicalShiftRight,
    ArithmeticShiftLeft, ArithmeticShiftRight,
    // expression forms with distinct semantics
    Conditional, Concatenation, Replication, Streaming,
    ElementSelect, RangeSelect, MemberAccess, Call, Conversion,
    Assignment, Inside, AssignmentPattern, ValueRange, MinTypMax,
    TaggedUnion, Other
};
```

例如：

- `a + b`：`DFGPrim(type=Operator, operatorType=Add)`
- `a == b`：`DFGPrim(type=Operator, operatorType=Equality)`
- `a ? b : c`：`DFGPrim(type=Operator, operatorType=Conditional)` 或专用 `Mux` prim
- `a & b`：`DFGPrim(type=Operator, operatorType=BinaryAnd)`；如果综合成 gate，可另建 `Gate` prim
- `a[3:0]`：`DFGPrim(type=Operator, operatorType=RangeSelect)`

## 4. DFGNode 与 DFGPort

### 4.1 DFGNode 没有 type

`DFGNode` 只记录 link：

| 字段 | 含义 |
| :--- | :--- |
| `linkId` + `linkKind` | 指向 `DFGGraph` 或 `DFGPrim` |
| `parentGraph` | 所属 graph |
| `pinHead` | 该 node 的 pin 链表 |
| `source` | 源位置 |

要判断 node 是什么：

```text
DFGNode.link -> DFGGraph(Module/Interface/Signal/Process/...)
             -> DFGPrim(Gate/Operator/Cell/Const/...)
```

- module instance：link 到 `DFGGraph(Module)`；
- 加减乘除：link 到 `DFGPrim(Operator, operatorType=Add/...)`；
- 与或非 gate：link 到 `DFGPrim(Gate)`；
- 常量：link 到 `DFGPrim(Const)`；
- signal value：link 到 `DFGGraph(Signal)`；
- 端口不是 node：使用 `DFGPort`。

### 4.2 原 DFGNodeType 的归属

旧 `DFGNodeType` 已删除。原来靠 node type 表示的信息，按“是否可展开/是否有 body”归到 Graph、Prim 或 DFGPort：

| 原 NodeType | 新归属 | 理由 |
| :--- | :--- | :--- |
| `GraphInstance` | 无枚举；node link `DFGGraph` | node 本身不是类型；类型由 graph 提供 |
| `PrimInstance` | 无枚举；node link `DFGPrim` | 类型由 prim 提供 |
| `Signal` | `DFGGraphType::Signal` | 信号是引用/值定义，可被 node link |
| `Port` | `DFGPort` | 端口是 graph 的边界对象，有独立 io pin，不是 node |
| `Constant` | `DFGPrimType::Const` | 常量是无 body 叶子 |
| `Operator` | `DFGPrimType::Operator` + `DFGOperatorType` | 加减乘除/比较等叶子操作，需细分 operatorType |
| `Mux` | `DFGPrimType::Mux` | 数据选择是叶子原语；也可用 Operator Conditional |
| `Register` | `DFGPrimType::Register` | 时序叶子，无 body |
| `Assign` | `DFGGraphType::Assign` | 有表达式 body，可展开/保存行为 |
| `Process` | `DFGGraphType::Process` | always/initial 有 body |
| `Control` | `DFGGraphType::Control` | if/case/loop 有控制流 body |
| `Blackbox` | `DFGGraphType::Blackbox` 或 `DFGPrimType::Blackbox` | 未解析 module 用 graph；未解析叶子用 prim |

判定原则：

- **有 body / 可展开 / 需要保存行为** → `DFGGraphType`（Module/Interface/Assign/Process/Control/...）；
- **无 body / 叶子语义** → `DFGPrimType`（Gate/Operator/Const/Register/Cell/...）；
- **graph 边界端口** → 独立 `DFGPort`，不塞进 node type；
- **node 是什么** → 只看 `linkKind + link`，不看 node 字段。

### 4.3 DFGPort

`DFGPort` 是 Graph 上的端口定义，类似 ModulePort：

- 属于 `DFGGraph`；
- 有 direction/type；
- **内置 io pin**（`ioPin`）；
- edge 可以连接 io pin；
- graph instance 的 pin 对应 graph port definition；
- port 本身不是 DFGNode。

`DFGGraph` 持有 `portHead`、`nodeHead`、`edgeHead`；`DFGPort` 与 `DFGNode` 分离。

## 5. DFGPin / DFGEdge

### 5.1 DFGPin

```cpp
enum class DFGPinDirection { Input, Output, InOut, Internal };

struct DFGPin {
    ObjectId ownerNode;      // set for a node pin
    ObjectId ownerPort;      // set for a DFGPort io pin
    DFGPinDirection direction;
    DFGType type;            // width, signed, four-state
    ObjectNameId name;
    uint32_t index;
};

struct DFGStmt;   // 行为控制流 side table，见第 9 节
struct DFGExpr;   // 行为表达式 side table，见第 9 节
```

- 一个 `DFGNode` 可以有多个 pin；
- GraphInstance 的 pins 对应 `DFGGraph` 的端口定义；
- PrimInstance 的 pins 对应 `DFGPrim` 的端口；
- Signal 节点可以有一个内部 pin 或作为 edge 的端点；
- pin 的 width/signed 必须与 edge 一致；vector/select 需要显式 slice。

### 5.2 DFGEdge

```cpp
enum class DFGEdgeKind {
    Data,
    Clock,
    Reset,
    Control,
    Assign,
    Alias,
    Unknown
};

struct DFGEdge {
    ObjectId srcPin;
    ObjectId dstPin;
    DFGType type;
    DFGEdgeKind kind;
};
```

- Edge 是有方向的：output/source → input/sink；
- 一条 net 的 fanout 可以是多个 edge；
- 多驱动/alias 用 edge kind 或 Signal node 的 driver/load 表表达；
- edge/pin 都有 width/signed，避免后续 optimize 时重新推类型。

## 6. Graph 与 Prim 的展开

### 6.1 DFGGraph

`DFGGraph` 是定义/引用：

- 有 graph ports（DFGPort）；
- 有 body nodes/edges；
- 可以被多个 `DFGNode(link=DFGGraph)` 引用；
- 展开时创建 occurrence body，把 graph ports 绑定到 node pins；
- 参数化 graph 需要 specialization key（参数值组合）或 unique graph。

```text
DFGGraph(Module: top)
  ├─ DFGPort list
  ├─ DFGNode(link=DFGGraph(Module: u0))
  │    ├─ pins
  │    └─ link -> DFGGraph(Module: child)
  └─ DFGEdge(...)
```

### 6.2 DFGPrim

`DFGPrim` 不可继续展开：

- 只有 ports/pins、primType、功能/时序属性；
- 一个 AND gate 是 `DFGPrim(type=Gate, gateType=And)`；
- 一个 LibCell 是 `DFGPrim(type=Cell, libCellId=...)`；
- 黑盒是 `DFGPrim(type=Blackbox)`。

```text
DFGNode(link=DFGPrim(Gate, And))
  ├─ pin A
  ├─ pin B
  ├─ pin Y
  └─ link -> DFGPrim(type=Gate, operatorType=And)
```

## 7. 与现有 logic DB 的对应

| 现有对象 | DFG 对应 |
| :--- | :--- |
| `Module` | `DFGGraph(type=Module)` |
| `HInst` | `DFGNode(link=DFGGraph(Module))` |
| `Inst`（LibCell） | `DFGNode(link=DFGPrim(Cell))` |
| `Inst`（primitive） | `DFGNode(link=DFGPrim(Gate))` |
| `ModulePort` | `DFGPort` + io `DFGPin` |
| `Net` | `DFGNode(link=DFGGraph(Signal))` + edges；或 flatten 后的 Signal |
| `HNet` | 某 HInst 内的 Signal occurrence |
| `Pin` | `DFGPin` |
| `HPin` | GraphInstance 的边界 `DFGPin` |
| `HInst` child list | GraphInstance body 内的 GraphInstance 节点 |
| `Net/HNet` 连接 | `DFGEdge` |
| `InstType` | `DFGGraphType` / `DFGPrimType` / `DFGOperatorType` |

迁移建议：

1. **DFG 作为 canonical semantic graph**；
2. 现有 `Module/HInst/Inst/Net/Pin` 暂时保留为 netlist view/adapter；
3. 不要长期维护两套不同拓扑；
4. 最终让 Net/Pin/HNet 从 DFGNode/DFGPin/DFGEdge 投影出来，或把 DFG 作为唯一真源。

## 8. Slang 信息如何吸收

| slang | DFG 目标 | 说明 |
| :--- | :--- | :--- |
| `DefinitionSymbol(Module)` | `DFGGraph(type=Module)` | graph 定义 |
| `DefinitionSymbol(Interface/Program)` | `DFGGraph(type=Interface/Program)` | 独立类型 |
| `InstanceSymbol(Module)` | `DFGNode(link=Module graph)` | 无 node type |
| `InstanceBodySymbol` | 该 GraphInstance 的 body/occurrence | 不是 Module 定义 |
| `InstanceArraySymbol` | GraphInstance node array | 保留 array path/索引 |
| `PortSymbol` | `DFGPort` + io pin | direction/type/width |
| `PortConnection` | `DFGEdge` | 父层连接；表达式先 lower 成 node/edge |
| `NetSymbol` | `DFGNode(link=Graph(Signal))` | wire/net |
| `VariableSymbol` | `DFGNode(link=Graph(Signal))` | reg/logic/var |
| `ParameterSymbol` | graph attributes + Constant node | 参数值/类型 |
| `Type` | `DFGType` | width/signed/dimensions，不作为 node |
| `ContinuousAssignSymbol` | `DFGNode(link=Graph(Assign))` 或 Assign graph | LHS/RHS 表达式变成 nodes/edges |
| `ProceduralBlockSymbol` | `DFGNode(link=Graph(Process))` 或 Process graph | 控制流/时序 |
| `Statement` | `DFGNode(link=Graph(Process))` + `DFGStmt` / control node | if/case/loop 不能只用 data edge |
| `Expression` | node link 到 Graph/Operator/Signal/Const prim | 具体 operator 由 `DFGOperatorType` 区分 |
| `GenerateBlockSymbol/Array` | `DFGGraph(type=GenerateBlock)` | 可展开 scope graph |
| `PrimitiveInstanceSymbol` | `DFGNode(link=DFGPrim)` | 无 node type |
| `PrimitiveSymbol` | `DFGPrim(type=Gate/Udp/Cell)` | 不可展开 |
| `PackageSymbol` | `DFGGraph(type=Package)` | 类型/常量引用 |
| `InterfacePortSymbol/Modport` | `DFGGraph(type=Interface)` + port/pin | 后续完整支持 |
| `UninstantiatedDefSymbol` | `DFGGraph(type=Blackbox)` + `DFGNode(link=Blackbox graph)` | 带诊断 |
| `NetAliasSymbol` | `DFGEdge(kind=Alias)` / Signal union | 别名 |
| `SourceLocation` | `SourceId` | 复制位置，不存 slang 指针 |
| `Expression`/`Statement` AST | 不导入 | 只导入转换后的 DFG node/edge/IR |

## 9. 行为信息的完整性

纯 DFG edge 可以表达组合数据流，但不能自动表达：

- `if/case` 分支；
- `for/while/repeat`；
- blocking/non-blocking assignment；
- 时序控制；
- 多驱动/优先级；
- process 的 sensitivity。

因此建议：

- `DFGNode(link=Graph(Process))` 持有一个 `DFGStmt` 控制流树或子图；
- 控制流节点通过 `DFGNode(link=Graph(Control))`/`DFGStmt` 表示；
- `DFGNode(link=Graph(Assign))` 保存 LHS/RHS 表达式图；
- `DFGEdge` 只表示确定的 data/control 连接；
- 复杂行为不要强行 flatten 成纯 edge。

这样才能做到“AST 不导入，但 RTL 完整行为信息在 DB”。

## 10. 推荐 type 枚举

```cpp
enum class DFGGraphType : uint8_t {
    Module, Interface, Program, Package, Function, Task,
    GenerateBlock, Process, Assign, Control, Signal, Blackbox, Top
};

enum class DFGPrimType : uint8_t {
    Gate, Mux, Register, Latch, Memory, Cell, Macro,
    Const, Operator, Udp, Blackbox
};

enum class DFGOperatorType : uint8_t {
    Unknown = 0,
    Plus, Minus, BitwiseNot, BitwiseAnd, BitwiseOr, BitwiseXor,
    BitwiseNand, BitwiseNor, BitwiseXnor, LogicalNot,
    Preincrement, Predecrement, Postincrement, Postdecrement,
    Add, Subtract, Multiply, Divide, Mod, Power,
    BinaryAnd, BinaryOr, BinaryXor, BinaryXnor,
    Equality, Inequality, CaseEquality, CaseInequality,
    WildcardEquality, WildcardInequality,
    GreaterThanEqual, GreaterThan, LessThanEqual, LessThan,
    LogicalAnd, LogicalOr, LogicalImplication, LogicalEquivalence,
    LogicalShiftLeft, LogicalShiftRight,
    ArithmeticShiftLeft, ArithmeticShiftRight,
    Conditional, Concatenation, Replication, Streaming,
    ElementSelect, RangeSelect, MemberAccess, Call, Conversion,
    Assignment, Inside, AssignmentPattern, ValueRange, MinTypMax,
    TaggedUnion, Other
};
```

`DFGGraphType` / `DFGPrimType` / `DFGOperatorType` 是对象种类和操作种类；`DFGType` 是位宽/signed 数据属性，二者不要混用。不要用一个巨大的 `uint32_t type`；用 enum + variant/subclass + side table，避免不同 type 的字段互相污染。

## 11. 关键风险

1. **Graph 身份**：Graph definition 与 GraphInstance 必须分开；否则共享/展开/unique 无法区分。
2. **参数特化**：parameterized Module 的 graph 可能需要 specialized graph；不能只按 definition 指针缓存。
3. **Signal/Edge 区分**：net 是多端对象，不能强行拆成单一 edge；用 Signal node + fanout edges 更合适。
4. **控制流完整性**：DFG 不能只存 data edges；always/if/case 需要控制流节点或 RTLStmt。
5. **类型作用域**：pin/edge width/signed 是数据属性；graph/prim 类型是对象种类，二者不要混为一个 type。
6. **AST 生命周期**：AST 只在 importer 内存在；DFG 全部使用 DB ID。
7. **多 top / interface / package**：要明确策略，不要隐式塞进 Module。

## 12. 与 slang 的对接顺序

### 第一阶段

- `DefinitionSymbol(Module)` → `DFGGraph(Module)`
- `InstanceSymbol` → `DFGNode(link=Graph(Module))`
- `PortSymbol`/`PortConnection` → graph pins / edges
- `NetSymbol`/`VariableSymbol` → Signal nodes
- `PrimitiveInstanceSymbol` → PrimInstance
- `PrimitiveSymbol` → Prim
- source/type/parameter 元数据

### 第二阶段

- ContinuousAssign → Assign node/graph + expression DFG
- Generate/instance arrays → graph/scope/array nodes
- Interface/package 基础支持

### 第三阶段

- ProceduralBlock → Process/Control/Stmt
- driver/load 分析
- flatten/synthesis 到 Net/Pin/LibCell

## 13. 当前代码落地

已在仓库中落地最小核心：

```text
include/DataModel/DFG/
  Enums.h
  Type.h
  DFGLink.h
  DFGGraph.h
  DFGPrim.h
  DFGNode.h
  DFGPort.h
  DFGPin.h
  DFGEdge.h
  MemoryMgr.h

src/DataModel/DFG/
  DFGGraph.cpp
  DFGPrim.cpp
  DFGNode.cpp
  DFGPort.cpp
  DFGPin.cpp
  DFGEdge.cpp
  MemoryMgr.cpp
```

当前实现：

- `DFGLink` → `DFGGraph` / `DFGPrim`；
- `DFGGraph` 持有 port/node/edge 链表，可展开；
- `DFGPrim` 为不可展开叶子，带 `DFGPrimType` 和 `DFGOperatorType`；
- `DFGNode` **不再有 node type**，通过 `linkId/linkKind` 指向 graph/prim；
- `DFGPort` 挂在 graph 上，内置 io pin；
- `DFGPin` 带方向、DFGType、edge list；
- `DFGEdge` 连接 src/dst pin，带 kind/DFGType；
- `DFGMemoryMgr` 管理 graph/prim/node/port/pin/edge 六张 ObjectTable；
- `Design::AttachDFG` / `GetDFG`；
- **只有 `DFGGraphType::Module` 的 graph 可以挂到 Design**；
- Design 销毁时递归销毁已挂载 graph tree；
- DFG 表接入 DesignMemoryMgr SaveAll/LoadAll/Clear；
- 测试覆盖 Module 挂载、Interface 拒绝、node/pin/edge、port/io pin、生命周期和 Save/Load。

尚未实现：

- graph/prim 的完整 port 绑定与 graph expansion API；
- DFG 与 Module/HInst/Inst 的双向投影；
- slang importer；
- DFG 行为控制流的具体结构。

```text
ctest 41/41
ASan/UBSan 41/41
默认构建通过
```

## 14. 结论

这个 DFG 模型比“直接扩 Module/HInst/Net/Pin”更统一：

- **所有可引用定义**统一为 `DFGLink`（Graph/Prim）；
- **所有 occurrence/行为节点**统一为 `DFGNode`，类型由 link 的 graph/prim 提供；
- **所有连接**统一为 `DFGEdge`；
- **所有节点端口**统一为 `DFGPin`；
- **所有 graph 端口**统一为 `DFGPort` + io pin；
- width/signed 统一为 `DFGType`；
- Graph 可展开，Prim 不可展开；
- slang AST 只做 importer 输入，DB 保存 DFG 语义图。

推荐把 DFG 作为后续 canonical semantic graph；现有 logic DB 对象作为 netlist view/adapter 逐步迁移，而不是继续把 RTL 信息硬塞进 `Module`/`Net`。
