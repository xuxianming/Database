# 字符串池采用追加式 interning，ID 不复用

对象名统一走一个池：相同字节 → 同一个 `NameId`；`0` 表示 invalid，`1` 表示空串；ID 在池内 append-only、永不复用。池只保存局部名（module / inst / net / port），层次路径名按需合成，不逐对象保存 flat path。P0 用 `std::deque<std::string>` + `std::unordered_map<std::string_view, NameId>` 实现，后续可在同一 API 下替换为 arena + 开放寻址。

理由：EDA 网表中 pin / port / 常量名高度重复，ID 等值比较和 name→object 查找依赖 canonical ID；对象表已经保存 `m_name_id`，ID 必须稳定并可随快照恢复。当前实现是按长度分块的分配器，没有 dedup，且内联 / 定长路径的解码有确定性错误，继续修补的成本高于替换。

代价：每个唯一名有一次 hash / 存储开销；v1 的名字生命周期 = 池生命周期，不支持 ECO 回收；32-bit `NameId` 最多约 4.29B 个唯一名；池非线程安全，写入需单线程或外部同步，Seal 之后只读。

## Considered Options

- **inline 短串编码**：省一次分配，但 ID 同时承载 flag / length / 字节，已出现 flag 与首字符冲突、长度解码错误和悬垂 view；而最长重复的短名（A / B / Y / CLK）恰好落在这里，不值得。
- **127 张定长表 + LCM 块大小**：LCM(1..127) 需要 183 bit，无法用 `size_t` 表示；每表独立块大小也会造成大块浪费，复杂度高于收益。
- **64-bit NameId**：容量更大，但对象与快照中的 ID 字节翻倍；当前规模 32-bit 足够，保留后续替换能力。
