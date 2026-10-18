> 模板来源：project-docs-workflow v2（模板演进后可重跑幂等 init 对齐；本文件可按项目实际修改）

# Database 上下文

Database 是 C++20 的 EDA 网表数据库库（参考 OpenDB/iEDA）；本文件是该 context 项目特定概念与词汇的单一事实源，不含实现细节。

## Language

### 设计聚合

**Design**：
一个完整用户设计的设计上下文，也是持久化单元；拥有 topmodule、名称池、SDC/MCMM、库引用和物理视图。Design 只持有 topmodule，不持有 HInst 链表；topmodule 持有顶层 HInst 并递归处理层次。Design 不维护 helper Module 表，unique Module 仍归原始 Design。没有实例化/嵌套概念。
_Avoid_: design instance、父 Design、Design 里的 Design

**current_design**：
当前工具活动、待展开/优化的 Design；大多数查询和修改以它为作用域。
_Avoid_: global design、默认 Design

**TopModule**：
Design 的顶层 Module，也是层次树的根；没有额外的 top HInst。PathName 以它为前缀。
_Avoid_: top design、root design

### 逻辑与层次

**Module**：
跨 Design 共享的只读 base 逻辑定义/容器（ports/Inst），可被实例化；同一个 Module 可以是 A 的 top、B 的 helper。基础 hierarchy expansion 不 uniquify，HInst 默认引用 base；需要设计私有修改时由 HInst uniquify 为 unique 副本；unique Module 保留原始 Module 的 Design 成员，不新建 Design。Module 没有顶层导航接口。
_Avoid_: design、hinst、层次节点、GetTopModule、默认复制 Module

**Inst**：
Module 内部声明的叶实例；目标是 LibCell；有局部名，不可展开。Module 实例化永远产生 HInst，不会产生可展开的 Inst。
_Avoid_: hinst、层次实例、可展开 Inst

**HInst**：
展开后的层次出现节点；`m_module_id` 永远指向 topmodule，`m_template_id` 指向实例化的 Module 定义；topmodule 持有顶层 HInst 列表；HInst 管理 child HInst、内部 Inst/HNet、边界 HPin 和上下层连接；未展开时内部链表为空。具有 parent/children/sibling 和唯一 PathName。`GetModule()` 直接返回 m_module_id，不循环；`GetHInst()` 返回上层 HInst（顶层为 null）。同一个 Module 可有多个 HInst。
_Avoid_: instance、module instance（单独使用时）、parent module

**Hierarchy expansion**：
永远基于一个 Module，以它为 template 递归创建 HInst；topmodule 持有顶层 HInst 列表并递归处理 HInst 树，HInst 管理 child HInst、内部 Inst/HNet/HPin。Design 不持有 HInst 链表，Module 不作为层次中的一层。
_Avoid_: design instantiation、展开 Design、Module 层次节点

**Flatten**：
为物理阶段把层次打平为 occurrence 级对象的过程；物理数据只在展开/打平后的视图中有意义。
_Avoid_: design flatten、隐式展开

**PathName**：
从 topmodule 名沿 HInst 父链合成的层次路径（如 `top/u0/u1`）；只属于 HInst/view，不属于 Module 定义，不存 flat path。
_Avoid_: module path、stored path id

### 连接

**Net / Pin / Port**：
Pin 有两个连接成员 `m_net_id`/`m_hnet_id`；Net 只连接 Pin，HNet 连接本层级 Pin/HPin。Net 是 Design/topmodule 级唯一的逻辑网络，存储只在 topmodule 的 Net 表；它忽略 hierarchy，只连接 Pin，并承载 physical/parasitic 信息；ModulePort 通过 io Pin 连接。Net **不连接 HPin**；可有 0..N 个 HNet。
_Avoid_: module-local net、per-HNet physical net、Net→HPin

**HNet / HPin**：
HNet 是 Net 的层次代理；owner 为 Module（顶层 HNet）或 HInst（中间 HNet），二者互斥。Net 每穿过一个 HInst 创建恰好一条 HNet；每个 HNet 唯一对应一个 Net；HNet 在所属 HInst 层级连接该层所有 Pin 和 HPin，以表达本层次完整连接，只承载逻辑层次信息。HNet 的 `GetModule` 返回 topmodule，`GetHInst` 返回所属 HInst；HPin 不提供 GetModule。
_Avoid_: physical HNet、独立网络、一个 HInst 多条 HNet、HPin::GetModule

### 导航

**GetDesign / GetModule / GetHInst**：
所有逻辑对象在创建/展开时缓存导航字段，直接返回，不沿 parent 链循环。`GetDesign` 返回顶层 Design（Module 返回原始 Design，unique 保留原 Design）；`GetModule` 返回 topmodule；`GetHInst` 返回所属/上层层次 HInst：HInst 返回上层 HInst，Inst/Net/HNet 返回所在 HInst；顶层 HInst 与 topmodule 直属对象返回 null。
_Avoid_: 循环向上找 top、Inst::GetModule 返回 owner Module、HNet::GetModule 返回 template Module

### 库、约束与物理

**TechLib / TimingLib**：
跨 Design 共享的只读 base 技术/时序库；Design 默认持有引用。需要设计私有修改时，通过 uniquify 接口创建 Design 私有的唯一化副本。
_Avoid_: 直接修改共享库、每个 Design 默认复制整库

**uniquify**：
把默认共享的 base 对象转为非共享 unique 副本的接口。unique 对象不再被其他 Design 共享；Module unique 保留原始 Design 成员，只由 HInst 引用；库 unique 副本归 Design 所有。

**SDC / MCMM / Scenario**：
Design 拥有的约束集合；MCMM 由 mode/corner/scenario 组合描述，TimingLib 提供共享 corner 数据。

**LEF Macro**：
TechLib 中的物理抽象；物理阶段 `Inst` 的 ref 扩展/绑定到 LEF Macro。

**Physical view / parasitic data**：
Design 在展开/打平后持有的布局、布线、寄生等物理信息；物理/寄生数据挂在 topmodule 级唯一的 `Net` 上，HNet 只做逻辑层次代理（每个穿过的 HInst 恰好一条）。

### 名称

**StringMgr / NameId**：
Design 私有的字符串驻留与名称 ID 池；ID/名称作用域跟随 Design。共享库对象使用库作用域的名称，不与 Design 私有名称池混用。
