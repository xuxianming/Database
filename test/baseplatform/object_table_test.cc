#include "BasePlatfrom/ObjectTable.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>

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

class ObjectTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove_all(kDir);
        std::filesystem::create_directories(kDir);
        Foo::live_count = 0;
    }

    void TearDown() override {
        EXPECT_EQ(Foo::live_count, 0);
        std::filesystem::remove_all(kDir);
    }
};

TEST_F(ObjectTableTest, Basics) {
    {
        db::ObjectTable<Foo, uint32_t, 7> table(1, kDir);
        EXPECT_EQ(table.Size(), 0u);
        EXPECT_EQ(table.Pointer(0), nullptr);
        EXPECT_GT(table.GetBlockSize(), 0u);

        const uint32_t id1 = table.GetNextObjId();
        Foo*           f1  = table.Make();
        ASSERT_NE(f1, nullptr);
        f1->SetObjectId(id1);
        f1->value = 111;

        const uint32_t id2 = table.GetNextObjId();
        Foo*           f2  = table.Make();
        ASSERT_NE(f2, nullptr);
        f2->SetObjectId(id2);
        f2->value = 222;

        EXPECT_TRUE(id1 != 0 && id2 != 0 && id1 != id2);
        EXPECT_EQ(table.Size(), 2u);
        EXPECT_EQ(Foo::live_count, 2);
        EXPECT_EQ(table.Pointer(id1), f1);
        EXPECT_EQ(table.Pointer(id2), f2);
        EXPECT_EQ(table.Pointer(id1)->value, 111u);
        EXPECT_EQ(table.Pointer(id2)->value, 222u);

        // Destroy one object: lifetime counter and size both drop.
        table.Destroy(id1);
        EXPECT_EQ(table.Size(), 1u);
        EXPECT_EQ(Foo::live_count, 1);

        // Double destroy must be a logged no-op, not corruption.
        table.Destroy(id1);
        EXPECT_EQ(table.Size(), 1u);
        EXPECT_EQ(Foo::live_count, 1);

        // Freed slot is reused by the next Make.
        const uint32_t id3 = table.GetNextObjId();
        EXPECT_EQ(id3, id1);
        Foo* f3 = table.Make();
        ASSERT_NE(f3, nullptr);
        f3->SetObjectId(id3);
        f3->value = 333;

        EXPECT_EQ(table.Size(), 2u);
        EXPECT_EQ(Foo::live_count, 2);
        EXPECT_EQ(table.Pointer(id3)->value, 333u);

        // Save -> Load roundtrip.
        EXPECT_TRUE(table.Save());
        EXPECT_TRUE(table.Load());
        EXPECT_EQ(table.Size(), 2u);
        // Load re-constructs the live slots before restoring their bytes, so
        // constructor/destructor accounting stays balanced.
        EXPECT_EQ(Foo::live_count, 2);
        EXPECT_EQ(table.Pointer(id2)->value, 222u);
        EXPECT_EQ(table.Pointer(id3)->value, 333u);

        // Clear destroys exactly the live objects and is idempotent.
        table.Clear();
        EXPECT_EQ(table.Size(), 0u);
        EXPECT_EQ(Foo::live_count, 0);
        table.Clear();
        EXPECT_EQ(table.Size(), 0u);
        EXPECT_EQ(Foo::live_count, 0);

        // Table is usable again after Clear; fresh blocks get fresh IDs.
        const uint32_t id4 = table.GetNextObjId();
        Foo*           f4  = table.Make();
        ASSERT_NE(f4, nullptr);
        f4->SetObjectId(id4);
        EXPECT_EQ(table.Size(), 1u);
        EXPECT_EQ(Foo::live_count, 1);
        table.Clear();
        EXPECT_EQ(Foo::live_count, 0);
    }

    // Destructor must release the table without a segfault or double-destroy.
    EXPECT_EQ(Foo::live_count, 0);
}

TEST_F(ObjectTableTest, NormalExitAfterClear) {
    // Same scenario again; the point is that normal scope exit does not crash.
    {
        db::ObjectTable<Foo, uint32_t, 7> table(2, kDir);
        // Empty table roundtrip.
        EXPECT_TRUE(table.Save());
        EXPECT_TRUE(table.Load());
        EXPECT_EQ(table.Size(), 0u);
        const uint32_t id = table.GetNextObjId();
        Foo*           f  = table.Make();
        ASSERT_NE(f, nullptr);
        f->SetObjectId(id);
        f->value = 42;
    }
    EXPECT_EQ(Foo::live_count, 0);
}

TEST_F(ObjectTableTest, AllFreeRoundtrip) {
    {
        db::ObjectTable<Foo, uint32_t, 7> table(3, kDir);
        const uint32_t id = table.GetNextObjId();
        Foo*           f  = table.Make();
        ASSERT_NE(f, nullptr);
        f->SetObjectId(id);
        table.Destroy(id);
        EXPECT_EQ(Foo::live_count, 0);
        EXPECT_EQ(table.Size(), 0u);

        EXPECT_TRUE(table.Save());
        EXPECT_TRUE(table.Load());
        EXPECT_EQ(table.Size(), 0u);
        EXPECT_EQ(Foo::live_count, 0);

        // Freed slots are still usable after the roundtrip.
        const uint32_t id2 = table.GetNextObjId();
        EXPECT_NE(id2, 0u);
        Foo* f2 = table.Make();
        ASSERT_NE(f2, nullptr);
        f2->SetObjectId(id2);
        EXPECT_EQ(table.Size(), 1u);
    }
    EXPECT_EQ(Foo::live_count, 0);
}

}  // namespace
