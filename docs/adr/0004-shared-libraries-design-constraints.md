# 库基础共享，按需 uniquify；SDC/MCMM 归 Design

TechLib/TimingLib 等库对象以跨 Design 的只读 base 形式共享；Design 默认持有引用，不复制。需要设计私有修改时，Design 调用 **uniquify** 接口创建该对象的 Design 私有副本，之后只修改副本；uniquify 后的对象归 Design 所有，并作为设计私有状态持久化。

SDC/约束归 Design 所有，支持 MCMM 的 mode/corner/scenario；TimingLib 提供共享 corner 数据。Design 快照记录共享库的版本/解析信息，保证完整设计可恢复；库内容内嵌到快照还是使用版本化外部依赖，在实现前另行决定。

理由：库通常体积大且被多个设计复用，默认复制会放大内存；完全可变的共享库又会导致跨 Design 串扰。共享 base + 按需 uniquify 兼顾复用和设计私有修改。

代价：需要维护 base/unique 的身份、版本和持久化边界；uniquify 后的对象可能增加内存；共享 base 不能被 Design 直接修改。

## Considered Options

- **每个 Design 复制完整库**：自包含但内存/加载成本高，未采用。
- **库完全可变且被 Design 共享**：会导致跨 Design 串扰、不可复现，未采用。
- **共享 base + uniquify**：默认低内存，修改时复制，采用。
