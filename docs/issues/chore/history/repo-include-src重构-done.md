# 仓库重构：include/ + src/ 与可安装动态库
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-18 ｜ 关联: 无

当前头文件与源文件混在根级模块目录（`BasePlatfrom/`、`DataModel/`、`Utils/`），CMake 只有一个未安装/未导出的 `database` 目标；外部模块无法 `find_package` 使用。需要：

- 公共头文件迁移到 `include/<Module>/...`，源文件迁移到 `src/<Module>/...`；`main.cc` -> `src/main.cc`。
- 单一共享库 `database`，构建接口只暴露 `include/`；安装头文件 + 动态库 + CMake package（`Database::database`）。
- 保留模块子目录，便于以后按模块拆仓；`DB_BUILD_TOOL`、`DB_BUILD_TESTS` 选项化。
- 清理导致外部可执行链接失败的 `HNet::ConnectPin/DisConnectPin/ConectHPin/DisConnectHPin` 未定义符号（提供定义）。
- 测试仍从 `test/` 使用公共 include；`db_ut` 37 用例、ctest 通过；默认不构建测试。

## 完成判据
- 根目录不再有 `BasePlatfrom/`、`DataModel/`、`Utils/` 源/头目录；`include/` 与 `src/` 分居。
- `target_include_directories(database PUBLIC include)`；安装导出 `DatabaseTargets.cmake` / `DatabaseConfig.cmake`。
- 外部 consumer 工程 `find_package(Database CONFIG REQUIRED)` + `target_link_libraries(... Database::database)` 可编译运行。
- `nm -uC libdatabase.so` 不再有本项目未定义符号。
- `./build.sh -c` 默认无测试；`./build.sh -c -t` 构建 `db_ut`；ctest 37/37。

## 结果
- 头文件全部迁移到 `include/<Module>/...`，源文件迁移到 `src/<Module>/...`，`main.cc` -> `src/main.cc`；旧根级模块目录与子 CMake 已移除。
- 重写顶层 CMake：单一 `database` 共享库；`include/` 为 PUBLIC build interface；`DB_BUILD_TOOL` / `DB_BUILD_TESTS` 选项化。
- 安装/导出：`Database::database` target、`DatabaseTargets.cmake`、`DatabaseConfig.cmake`、版本文件；安装头文件与共享库。
- 补齐 `HNet::ConnectPin/DisConnectPin/ConectHPin/DisConnectHPin` 定义，`nm -uC` 不再有项目自身未定义符号。
- 外部 consumer 验证通过：`find_package(Database CONFIG REQUIRED)` + `Database::database`，编译运行 StringMgr 示例。
- 测试与构建：`db_ut` 37/37、ctest 37/37；默认 `DB_BUILD_TESTS=OFF` 无测试；`./build.sh -c` / `./build.sh -c -t` 均通过；`DB_BUILD_TOOL=OFF` 可只构建库。
- `CLAUDE.md` 架构/构建说明已更新到 include/src 与安装导出。

## 步骤
- [x] 盘点/移动 include/src 与删除旧 CMake
- [x] 重写 CMake（共享库/安装/导出/选项）
- [x] 修复本地 include 与 HNet 符号
- [x] 构建、db_ut、ctest、install consumer 验证
- [x] 更新构建文档并归档

## Log
- 2026-10-18 用户要求 include/src 结构与动态库对外；创建条目。
- 2026-10-18 重构完成；外部 consumer/install/export 与全套测试通过；收尾归档。
