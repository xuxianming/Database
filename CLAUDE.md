# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

A C++20 EDA netlist database library (modeled on OpenDB/iEDA), plus a small SystemVerilog front-end tool built on [slang](https://github.com/MikePopoloski/slang). All library code lives in namespace `db`. The library produces `libdatabase.so`; the executable `my_tool` uses slang to elaborate a hardcoded Verilog file and dump AST/port info — it does **not** link the `database` library yet.

## Build & run

```bash
./build.sh            # configure + build (Release), output to build/
./build.sh -c         # clean build (deletes build/ first), no tests
./build.sh -c -t      # clean build including GoogleTest tests
./build.sh -c -T Debug # clean debug build (no tests)
./build/bin/my_tool   # run the slang demo (parses test/test.v)
```

- Outputs: `build/lib/libdatabase.so`, `build/bin/my_tool`; with `-t` also `build/bin/db_ut`.
- Windows: `build.bat` (auto-detects Ninja / MSVC / MinGW); `-t` builds tests, `-T <type>` sets Debug/Release.
- `my_tool` requires `slang` (`DB_BUILD_TOOL=ON`, default); set `-DDB_BUILD_TOOL=OFF` to build only the library. The library itself does not link slang.
- Tests use GoogleTest (system GTest 1.14), grouped under `test/baseplatform`, `test/design`, `test/liberty`; all sources link into one `build/bin/db_ut` executable (`test/main.cc` is the only main). `ctest --test-dir build --output-on-failure` runs them after a `-t` build. `test/test.v` is a Verilog fixture consumed by `my_tool`.
- Sources are collected by one top-level `file(GLOB_RECURSE ...)` over `src/`; newly added `.cpp` files require re-running cmake (the build scripts configure on every run).
- Public headers live in `include/`; `src/` is implementation-only. `cmake --install` installs the shared library, headers, and a CMake package; external projects use `find_package(Database CONFIG REQUIRED)` + `Database::database`.

## Architecture

The library is layered bottom-up:

### `include/BasePlatfrom` + `src/BasePlatfrom` — memory infrastructure (header-heavy templates)

- `StringMgr.h` — `StringMgr<IdType, ...>` interning pool: 64 KiB arena storage + stable `string_view` table + open-addressing FNV-1a index; IDs are append-only, `0` invalid, `1` empty; reference counted (`AddString`/`ReleaseString`), `Seal()` read-only contract, `DBST` v2 Save/Load.
- `ObjectTable.h` — `ObjectTable<TYPE, IdType, OBJECT_INDEX_BITS=7>`: raw-storage blocks with live/free state and a free list. An `ObjectId` encodes (block index << index bits | slot); `Make`/`Destroy` own placement-new/destruction; supports binary `Save()`/`Load()` (one `.db` file per type).
- `IdArrayMgr.h` — dynamic-only `IdArrayMgr<IdType, ValueType, FixLen>` + non-owning `IdArray` view for variable-length id collections (e.g. an instance's pin list); stable-ID `Save/Load`.

### `include/DataModel` + `src/DataModel` — the netlist/EDA object model

- `Type.h` — core ID typedefs (`ObjectId`, `FullObjectId`, `ObjectNameId`, …). `FullObjectId` packs a `DMObjectType` (8 bits, at `TypeOffset=56`) with the object id. The `_UINT_64_` macro toggles a 56-bit-id (64-bit build) vs 32-bit-id layout; it is **not** defined by CMake — pass `-D_UINT_64_` manually if needed.
- `Object.h/.cpp` — `Object` base class holding `m_type`, `m_obj_id`, `m_name_id`. `IsDesignObject()` / `IsLibertyObject()` are decided by comparing the type against boundary enum values.
- `Enums.h` — the `DBENUM(name, base, elements)` macro generates an `enum class` plus a `ToString()`. Defines `DMObjectType` (with `DESIGNOBJ_BOUNDARY` / `LIBOBJECT_BOUNDARY` / `PHYSICALOBJ_BOUNDARY` separators), `Direction`, `TimingType`, `LogicType`, `OperatorType`, `LogicPort`, etc.
- `Design/` — hierarchical design objects, all deriving from `Object`:
  - `Design` (top container: owns a string mgr + id-array mgr + module list), `Module` (netlist container), `Inst`, `HInst` (hierarchical *view* of a module instance), `Net`, `HNet`, `Pin`, `HPin`, `ModulePort`/`Port`, `BusPort`, `BusNet`.
  - Objects reference each other by `ObjectId` through **intrusive linked lists** (members named `*_head_id` / `*_next_id` / `*_prev_id`) rather than `std::vector` or raw pointers, for O(1) insertion/traversal without allocation. Iterators (e.g. `ModuleInstIterator`, `HInstInstIterator`) walk these lists; they're declared `friend` so they can reach the private link members.
  - `Design/MemoryMgr.h/.cpp` — `DesignMemoryMgr`: static factory/registry. One global `ObjectTable<>` per concrete type; `Create*` / `Get*` / `Destroy*` / `SaveAll` / `LoadAll`.
  - `Design/Type.h` — instantiates the mgrs: `DesignStringMgr = StringMgr<ObjectNameId,127,128,1>`, `DesignIdArrayMgr = IdArrayMgr<ObjectId,ObjectId,15>`, plus array-id typedefs.
- `Liberty/` — Liberty (.lib) representation: `LibertyLib`, `LibCell`, `LibPort`, with a *separate* `LibertyMemoryMgr` and its own string/id-array managers (`Liberty/Type.h`).
- `Tech/` — `Macro` (physical/tech objects, currently a stub).

### `include/Utils` + `src/Utils`

- `Utils/Log.h` — `Logger` singleton + `DB_LOG(LEVEL)` macro (`DB_LOG(ERROR) << ...`).
- `Utils/Unils.h` — constexpr LCM / perfect-size helpers.
- `Common/` and `Passer/` placeholders have been removed; new modules should add `include/<Module>` and `src/<Module>` directories.

## Conventions worth knowing

- Everything is in namespace `db`. Classes are `CamelCase`, members are `m_lower_case` (`.clang-format` is Google-based; `.clang-tidy` enforces a naming scheme with the `m_` prefix).
- `Object` subclasses use `friend` declarations heavily to let managers and iterators set private linked-list members — add the corresponding `friend` when introducing a new object type or iterator.
- To add a new object type: add it to the `DMObjectType` enum (respecting the `*_BOUNDARY` separators), give it a header deriving from `Object`, add a global `ObjectTable<>` + `Create*/Get*/Destroy*` entries in the relevant `MemoryMgr`, and (for `_UINT_64_` builds) keep the type field within the 8-bit `TypeOffset` budget.
