# 构建脚本：默认不编译测试，-t 编译测试
> Status: done ｜ 责任: agent ｜ 创建: 2026-10-18 ｜ 关联: 测试框架迁移（done）

当前 CMake 无条件 `add_subdirectory(test)`，`build.sh`/`build.bat` 没有测试开关。需要：
- CMake 增加 `DB_BUILD_TESTS`（默认 OFF），仅 ON 时 `enable_testing()` + `add_subdirectory(test)` + `find_package(GTest)`。
- `build.sh` / `build.bat` 默认 OFF；`-t` 开启测试。
- 由于 `-t` 原来是 build type 参数，改为 `-T <type>` 传 Debug/Release；帮助与示例同步。
- 保持默认构建只产出库/工具，不编译 gtest；`-t` 时构建 gtest 目标并提示 ctest 命令。

## 完成判据
- `./build.sh -c`：默认不生成测试目标/二进制。
- `./build.sh -c -t`：生成 10 个 gtest 目标，ctest 37/37。
- `build.bat -t` / `-T` 与 shell 语义一致；帮助文本更新。
- 默认 CMake 配置 `DB_BUILD_TESTS=OFF`；显式 ON 才找 GTest。

## 结果
- CMake 增加 `DB_BUILD_TESTS`（默认 OFF）；仅 ON 时 `enable_testing()`、`add_subdirectory(test)`、`find_package(GTest)`。
- `build.sh`：默认 OFF；`-t` 开启测试；`-T <type>` 传 Debug/Release；帮助/示例更新。
- `build.bat`：同样默认 OFF、`-t` 开关、`-T <type>`；帮助/CMake 参数更新。
- 验证：`./build.sh -c` 默认仅产出 `my_tool`，无测试二进制；`./build.sh -c -t -T Debug` 构建 10 个 gtest 目标，ctest 37/37。
- `CLAUDE.md` 的构建/测试说明同步更新。

## 步骤
- [x] CMake 条件化测试子目录
- [x] build.sh 参数与帮助
- [x] build.bat 参数与帮助
- [x] 默认/`-t` 两种构建验证
- [x] 收尾归档

## Log
- 2026-10-18 用户要求默认不编译测试、`-t` 编译测试；创建条目。
- 2026-10-18 实现完成；默认/测试两种构建验证通过；收尾归档。
