// Standalone tests for db::ObjectTable lifetime handling.
//
// Build & run:
//   cmake --build <build> --target object_table_test && ./bin/object_table_test

#include "BasePlatfrom/ObjectTable.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::fprintf(stderr, "CHECK failed: %s\n  at %s:%d\n", #cond,        \
                         __FILE__, __LINE__);                                    \
            std::exit(1);                                                        \
        }                                                                        \
    } while (false)

namespace {

constexpr const char* kDir = "/tmp/opencode/object_table_test";

// Polymorphic type: a virtual destructor is what used to crash when the free
// list clobbered the vptr.
struct Foo {
    Foo() { ++live_count; }
    virtual ~Foo() { --live_count; }

    uint32_t GetObjectId() const { return id; }
    void     SetObjectId(uint32_t value) { id = value; }

    uint32_t id{0};
    uint32_t value{0};

    static inline int live_count = 0;
};

void TestBasics() {
    std::filesystem::remove_all(kDir);
    std::filesystem::create_directories(kDir);

    {
        db::ObjectTable<Foo, uint32_t, 7> table(1, kDir);
        CHECK(table.Size() == 0);
        CHECK(table.Pointer(0) == nullptr);
        CHECK(table.GetBlockSize() > 0);

        const uint32_t id1 = table.GetNextObjId();
        Foo*           f1  = table.Make();
        CHECK(f1 != nullptr);
        f1->SetObjectId(id1);
        f1->value = 111;

        const uint32_t id2 = table.GetNextObjId();
        Foo*           f2  = table.Make();
        CHECK(f2 != nullptr);
        f2->SetObjectId(id2);
        f2->value = 222;

        CHECK(id1 != 0 && id2 != 0 && id1 != id2);
        CHECK(table.Size() == 2);
        CHECK(Foo::live_count == 2);
        CHECK(table.Pointer(id1) == f1);
        CHECK(table.Pointer(id2) == f2);
        CHECK(table.Pointer(id1)->value == 111);
        CHECK(table.Pointer(id2)->value == 222);

        // Destroy one object: lifetime counter and size both drop.
        table.Destroy(id1);
        CHECK(table.Size() == 1);
        CHECK(Foo::live_count == 1);

        // Double destroy must be a logged no-op, not corruption.
        table.Destroy(id1);
        CHECK(table.Size() == 1);
        CHECK(Foo::live_count == 1);

        // Freed slot is reused by the next Make.
        const uint32_t id3 = table.GetNextObjId();
        CHECK(id3 == id1);
        Foo* f3 = table.Make();
        CHECK(f3 != nullptr);
        f3->SetObjectId(id3);
        f3->value = 333;

        CHECK(table.Size() == 2);
        CHECK(Foo::live_count == 2);
        CHECK(table.Pointer(id3)->value == 333);

        // Save -> Load roundtrip.
        CHECK(table.Save());
        CHECK(table.Load());
        CHECK(table.Size() == 2);
        // Load re-constructs the live slots before restoring their bytes, so
        // constructor/destructor accounting stays balanced.
        CHECK(Foo::live_count == 2);
        CHECK(table.Pointer(id2)->value == 222);
        CHECK(table.Pointer(id3)->value == 333);

        // Clear destroys exactly the live objects and is idempotent.
        table.Clear();
        CHECK(table.Size() == 0);
        CHECK(Foo::live_count == 0);
        table.Clear();
        CHECK(table.Size() == 0);
        CHECK(Foo::live_count == 0);

        // Table is usable again after Clear; fresh blocks get fresh IDs.
        const uint32_t id4 = table.GetNextObjId();
        Foo*           f4  = table.Make();
        CHECK(f4 != nullptr);
        f4->SetObjectId(id4);
        CHECK(table.Size() == 1);
        CHECK(Foo::live_count == 1);
        table.Clear();
        CHECK(Foo::live_count == 0);
    }

    // Destructor must release the table without a segfault or double-destroy.
    CHECK(Foo::live_count == 0);
    std::filesystem::remove_all(kDir);
}

void TestNormalExitAfterClear() {
    // Same scenario again; the point is that normal scope exit does not crash.
    std::filesystem::remove_all(kDir);
    {
        db::ObjectTable<Foo, uint32_t, 7> table(2, kDir);
        // Empty table roundtrip.
        CHECK(table.Save());
        CHECK(table.Load());
        CHECK(table.Size() == 0);
        const uint32_t id = table.GetNextObjId();
        Foo*           f  = table.Make();
        CHECK(f != nullptr);
        f->SetObjectId(id);
        f->value = 42;
    }
    CHECK(Foo::live_count == 0);
    std::filesystem::remove_all(kDir);
}

void TestAllFreeRoundtrip() {
    std::filesystem::remove_all(kDir);
    {
        db::ObjectTable<Foo, uint32_t, 7> table(3, kDir);
        const uint32_t id = table.GetNextObjId();
        Foo*           f  = table.Make();
        CHECK(f != nullptr);
        f->SetObjectId(id);
        table.Destroy(id);
        CHECK(Foo::live_count == 0);
        CHECK(table.Size() == 0);

        CHECK(table.Save());
        CHECK(table.Load());
        CHECK(table.Size() == 0);
        CHECK(Foo::live_count == 0);

        // Freed slots are still usable after the roundtrip.
        const uint32_t id2 = table.GetNextObjId();
        CHECK(id2 != 0);
        Foo* f2 = table.Make();
        CHECK(f2 != nullptr);
        f2->SetObjectId(id2);
        CHECK(table.Size() == 1);
    }
    CHECK(Foo::live_count == 0);
    std::filesystem::remove_all(kDir);
}

}  // namespace

int main() {
    TestBasics();
    TestNormalExitAfterClear();
    TestAllFreeRoundtrip();
    std::printf("object_table_test: all checks passed\n");
    return 0;
}
