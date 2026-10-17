> 模板来源：project-docs-workflow v2（模板演进后可重跑幂等 init 对齐；本文件可按项目实际修改）

# 开发流程（推荐）

本文件是推荐的单 issue 开发流程；项目可按实际覆盖。核心：小步推进、随时可 review、与用户对齐后再动手。

## 工作单元

一条 issue = 一个可独立交付的工作单元（通常 feature 级；bug / chore 同构，形态相应简化）。

粒度建议（非强制）：切到「可单次 review 的推进单元」，≈0.5 人天仅启发式参考，按任务弹性调整。工作如何切分由 agent 自行感知，阶段不落盘（必要时可在当前 issue 内记一句）。

## 开工前对齐

读 PDR 目标（`docs/pdr/`）与 Current 现状（`docs/current/`），列出差距候选（模块 / FR、类型、切片、完成判据），与用户确认要做哪条并落 issue。差异只是候选：可能是漂移 / 质量 / 未验证 / 目标过时，不都是缺失功能。

- 对齐范围内 agent 自主实现（遵守 ADR / 契约 / 安全）；高成本、破坏性或难以回退的决定先与用户确认。
- 对话确认即可：不冻结、不设仪式行。
- 需求变化时主动重对齐，并更新 issue 内的计划与完成判据。
- 方向有歧义 → 调用 `grilling`（未安装则按 `docs/agents/deps/grilling.md` 执行）；`grill-me` 仅入口别名。
- 不要吞并超出对齐范围的工作。

## 五步流程（规划 / 骨架 / 细节 / 异常 / 回顾）

### 规划

细化完成判据与步骤状态，给出 1–2 句方案。产出最小，不新开章节。PDR 目标与 Current 现状的差距分析在此完成（对照 `docs/pdr/` 与 `docs/current/`；候选带模块 / FR、类型、切片、完成判据；用户挑选后才落 issue）。

### 骨架

实现最小可运行的垂直切片（忽略输出 / 格式细节），留下一条运行证据，并把证据展示给用户（首个 review 点）。纯修复可跳过（在 Log 注明理由）；feature / 结构性改动必做。

### 细节

主路径实现、细节打磨与代码质量。`tdd` 可作内部循环（可选；未安装则按 `docs/agents/deps/tdd.md` 执行）。

### 异常

两点含义：作为流程步骤，主路径完成后再处理失败路径、边界与稳定性；作为开发现实，真实的开发阻塞立即处理并记 Log，不排队到本步。

### 回顾（收尾回顾）

issue 完成时做一次轻量的文档同步清单（`pdr`、`current`、`adr`、`context`、`issues`、`agents` 是否有需更新）；若无，一行 Log 即可。仅做文档同步；会话复盘属于 `retro`（未安装则按 `docs/agents/deps/boundaries.md` 执行）。

## 进度与续接

- 步骤记录在 issue 的 `## 步骤` 清单（就地更新）；仅在关键节点（范围变化 / 阻塞 / 交接 / 放弃）记 Log，不需要逐步机械追加。
- session 为软约束：session = 用户注意力窗口，节奏由用户掌握；agent 不得吞并超出对齐范围的工作。
- 恢复时先读 issue 的 `## 步骤` 与 `## Log`，与用户确认续点后再继续。
- issue 收尾时对照变更检查 Current：若变更已包含在选定基线内则更新 `docs/current/` 对应文件（只改事实，不插入指向 issue 的自链接或任务历史），否则不更新，在 issue 记待同步项；并按 `issues.md` 归档。

## 边界

- 高风险 / 大规模多阶段 → `deepwork`（未安装则按 `docs/agents/deps/boundaries.md` 执行），不叠加。
- `tdd` 可作细节内循环（seam 确认并入规划对齐；未安装则按 `docs/agents/deps/tdd.md` 执行）。
- `grilling` 为对齐访谈（`grill-me` 仅入口别名；未安装则按 `docs/agents/deps/grilling.md` 执行）。

## 相关

- Issue 体系：`issues.md`
- PDR 目标文档集：`../pdr/00-README.md`
- 现状文档集（Current）：`../current/00-README.md`
- 依赖兜底文档（外部技能未安装时）：`docs/agents/deps/`
- 文档 HTML 渲染（可选、手动）：独立技能 `docs-html-view`；本流程不自动安装 / 构建 / 渲染。
