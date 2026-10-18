# Module base 共享，HInst 按需 uniquify

Module 定义默认以只读 base 形式跨 Design 共享：同一个 Module 可以是 Design A 的 topmodule，同时是 Design B 的 helper module。基础 hierarchy expansion 只建立 HInst 对 base Module 的引用，不做 uniquify。

需要设计私有修改（改名、增量对象、物理/优化状态等）时，HInst 通过 **uniquify** 接口把 base Module 转为 unique 副本；之后只修改副本。unique Module **保留原始 Module 的 Design 成员，不新建 Design**，由原始 Design 管理，并被 HInst 的 template 引用；它随 HInst 可达树一起持久化，base Module 保持只读。

理由：Module 在真实设计中大量复用，默认复制会让内存和展开成本不可控；完全可变的共享 Module 又会造成跨 Design 串扰。base + 按需 uniquify 兼顾共享和私有修改。

代价：需要维护 base/unique Module 的身份、名称/ID 作用域和版本；unique 副本增加内存/磁盘；HInst 必须记录它引用的是 base 还是 unique 副本；Design 不持有 helper Module 表，unique Module 仍归原始 Design，持久化需遍历 HInst 树收集可达 Modules。

## Considered Options

- **基础展开就复制 Module**：展开成本高，未采用。
- **所有 Module 全局可变**：跨 Design 串扰、不可复现，未采用。
- **base 共享 + HInst uniquify**：默认低内存，修改时复制，采用。

## Log

- 2026-10-19 修正 unique Module 归属：保留原始 Design，不新建 Design。
