# 测试迁移到 gtest 并按模块分组
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-18 ｜ 关联: 无

当前 `test/` 下 10 个测试文件平铺，各自带自定义 `main` + `CHECK` 宏。系统已安装 GTest 1.14（`libgtest-dev`，CMake `find_package(GTest)` 可用）。需要：
- 全部测试改用 gtest（`TEST`/`EXPECT_*`/`ASSERT_*`，不保留自定义 main/CHECK）。
- `test/` 按模块分组：`baseplatform/`、`design/`、`liberty/`。
- CMake 使用 `test/CMakeLists.txt` 管理测试；每个原测试文件保持独立可执行文件（避免跨文件全局 ObjectTable 状态污染），用 `gtest_discover_tests` 注册。
- 保留 `test/test.v`（`main.cc` 硬编码路径）与现有测试覆盖。

## 完成判据
- 10 个测试文件全部使用 gtest；无 `CHECK` 宏 / 自定义 `main`。
- 目录结构按模块分组；顶层 CMake 不再逐个声明测试。
- `find_package(GTest REQUIRED)` + `GTest::gtest_main` + `gtest_discover_tests`。
- 全量构建、ctest 通过；gtest 用例数与覆盖不减少。

## 结果
- 系统 GTest 1.14 已安装；CMake `find_package(GTest REQUIRED)` 成功，使用 `GTest::gtest_main` + `gtest_discover_tests`。
- `test/` 重组为 `baseplatform/`（StringMgr/ObjectTable/IdArrayMgr）、`design/`（6 个）、`liberty/`（1 个）；`test/test.v` 保持在根目录供 `main.cc` 使用。
- 10 个测试文件全部改写为 gtest；无自定义 `main` / `CHECK` / `std::exit`。每个原文件保持独立可执行，避免全局 ObjectTable 状态跨文件污染。
- 顶层 CMake 移除逐个测试目标，改为 `add_subdirectory(test)`；`test/CMakeLists.txt` 统一声明 10 个 gtest 目标。
- 验证：完整构建通过；ctest 37/37 通过；各测试二进制直接运行通过。

## 步骤
- [x] 规划（目录、CMake、每文件独立可执行）
- [x] 迁移 baseplatform 测试
- [x] 迁移 design 测试
- [x] 迁移 liberty 测试
- [x] 构建/ctest 验证与收尾

## Log
- 2026-10-18 检查系统 GTest 1.14 已安装；创建迁移条目与目录骨架。
- 2026-10-18 10 个测试文件迁移完成；ctest 37/37；收尾归档。
