# 测试合并为单一 db_ut 可执行
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-18 ｜ 关联: 测试框架迁移（done）

当前 10 个测试文件各自生成可执行。需要：
- 所有测试源编译成一个 `db_ut` 可执行文件。
- `test/` 下只有一个 `main`（`test/main.cc`），负责 `InitGoogleTest` + `RUN_ALL_TESTS`。
- 所有用例统一使用 `TEST_F`（每个 suite 有 fixture 类）。
- CMake 链接 `GTest::gtest`（不是 `gtest_main`），`db_ut` 通过 `gtest_discover_tests` 注册。
- 单进程运行要保证跨测试的全局 ObjectTable/Design/Liberty 状态被清理/隔离。

## 完成判据
- 构建只产生 `db_ut` 一个测试可执行（不再有 `*_test` 测试目标）。
- `grep` 无 `TEST(`，只有 `TEST_F(`；`test/` 只有一个 `main`。
- `db_ut` 37 个用例全过；`ctest` 通过；打乱顺序/重复运行不因全局状态失败。
- 默认构建仍不编译测试；`-t` 构建 `db_ut`。

## 结果
- 新增 `test/main.cc` 作为唯一 `main`（InitGoogleTest + RUN_ALL_TESTS）；所有 37 个用例统一为 `TEST_F`，每 suite 一个 fixture。
- `test/CMakeLists.txt` 改为单一 `db_ut` 目标，链接 `GTest::gtest`（非 gtest_main），`gtest_discover_tests` 注册 37 个用例。
- 单进程测试隔离：`test/main.cc` 注册 per-test listener，在每个用例结束后调用 `DesignMemoryMgr::Clear()`、`LibertyMemoryMgr::Clear()` 并删除生成的 DB 快照；补齐 `DesignMemoryMgr::Clear()` 实现。
- `design_name_test` 补齐所有 Module 销毁；其他测试保留原有清理。
- 验证：`db_ut` 直接运行 37/37；`--gtest_shuffle --gtest_repeat=3` 全过；ctest 37/37；`./build.sh -c -t -T Debug` 只生成 `build/bin/db_ut`（另有 `my_tool`）；默认 `DB_BUILD_TESTS=OFF` 只生成 `my_tool`。
- 全部测试源 `TEST(` 为 0，`test/` 只有一个 `main`。

## 步骤
- [x] 添加 test/main.cc，统一 TEST_F fixtures
- [x] CMake 单目标 db_ut
- [x] 清理跨测试全局状态
- [x] 构建/ctest/shuffle 验证
- [x] 收尾归档

## Log
- 2026-10-18 用户要求所有测试合并为 db_ut，test 下单一 main，使用 TEST_F；创建条目。
- 2026-10-18 实现完成；db_ut 37/37、shuffle/repeat、ctest 通过；收尾归档。
