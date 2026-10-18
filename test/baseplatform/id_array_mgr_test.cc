#include "BasePlatfrom/IdArrayMgr.h"
#include "DataModel/Design/Type.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using Mgr = db::IdArrayMgr<uint32_t, uint32_t, 15>;

class IdArrayMgrTest : public ::testing::Test {};

void Fill(db::IdArray<uint32_t, uint32_t>& array, uint32_t seed) {
    for (size_t i = 0; i < array.Capacity(); ++i) {
        array.PushBack(seed + static_cast<uint32_t>(i));
    }
}

bool Verify(const db::IdArray<uint32_t, uint32_t>& array, uint32_t seed) {
    for (size_t i = 0; i < array.Size(); ++i) {
        if (array[i] != seed + static_cast<uint32_t>(i)) {
            return false;
        }
    }
    return true;
}

std::string SaveToBytes(const Mgr& mgr) {
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    EXPECT_TRUE(mgr.Save(ss));
    return ss.str();
}

bool LoadFromBytes(Mgr& mgr, const std::string& bytes) {
    std::istringstream in(bytes, std::ios::in | std::ios::binary);
    return mgr.Load(in);
}

void CheckManagerEmpty(Mgr& mgr) {  // after a failed load
    EXPECT_EQ(mgr.Get(0).Size(), 0u);
    EXPECT_THROW((void)mgr.Get(1), std::out_of_range);
}

TEST_F(IdArrayMgrTest, EmptyAndZeroId) {
    Mgr  mgr;
    auto empty = mgr.Allocate(0);
    EXPECT_EQ(empty.Size(), 0u);
    EXPECT_EQ(empty.Id(), 0u);

    EXPECT_EQ(mgr.Get(0).Size(), 0u);
    EXPECT_EQ(mgr.Get(0).Id(), 0u);
    mgr.Free(0);  // no-op
}

TEST_F(IdArrayMgrTest, Lengths) {
    const std::vector<size_t> lengths = {1, 3, 7, 15, 16, 100, 1000, 65535};
    Mgr                       mgr;

    struct Rec {
        uint32_t id;
        size_t   len;
    };
    std::vector<Rec> recs;

    for (size_t i = 0; i < lengths.size(); ++i) {
        const size_t len = lengths[i];
        auto         arr = mgr.Allocate(len);
        EXPECT_EQ(arr.Capacity(), len);
        EXPECT_NE(arr.Id(), 0u);
        Fill(arr, static_cast<uint32_t>(i * 1000 + 1));
        EXPECT_EQ(arr.Size(), len);
        recs.push_back({arr.Id(), len});
    }

    // Views still resolve to the same ids/data through Get().
    for (size_t i = 0; i < recs.size(); ++i) {
        auto view = mgr.Get(recs[i].id);
        EXPECT_EQ(view.Id(), recs[i].id);
        EXPECT_EQ(view.Size(), recs[i].len);
        EXPECT_EQ(view.Capacity(), recs[i].len);
        EXPECT_TRUE(Verify(view, static_cast<uint32_t>(i * 1000 + 1)));
    }

    // Free -> Get must reject the id; double Free must throw.
    const uint32_t freed_id = recs[1].id;
    mgr.Free(freed_id);
    EXPECT_THROW((void)mgr.Get(freed_id), std::out_of_range);
    EXPECT_THROW(mgr.Free(freed_id), std::runtime_error);

    // Out-of-range ids and oversized requests are rejected.
    EXPECT_THROW((void)mgr.Get(999999), std::out_of_range);
    EXPECT_THROW(mgr.Free(999999), std::out_of_range);
    EXPECT_THROW((void)mgr.Allocate(65536), std::runtime_error);
}

TEST_F(IdArrayMgrTest, ExactReuse) {
    Mgr mgr;

    auto           first = mgr.Allocate(16);
    const uint32_t id    = first.Id();
    Fill(first, 1111);
    mgr.Free(id);

    auto reused = mgr.Allocate(16);
    EXPECT_EQ(reused.Id(), id);
    EXPECT_EQ(reused.Capacity(), 16u);
    Fill(reused, 2222);
    EXPECT_EQ(reused.Size(), 16u);
    EXPECT_TRUE(Verify(mgr.Get(id), 2222));
}

TEST_F(IdArrayMgrTest, Split) {
    Mgr mgr;

    auto           big    = mgr.Allocate(1000);
    const uint32_t big_id = big.Id();
    Fill(big, 7);
    mgr.Free(big_id);

    auto small = mgr.Allocate(100);
    EXPECT_EQ(small.Id(), big_id);  // larger free block is split in place
    EXPECT_EQ(small.Capacity(), 100u);
    Fill(small, 9000);
    EXPECT_EQ(small.Size(), 100u);

    auto remainder = mgr.Allocate(899);  // 1000 - 100 - length word
    EXPECT_EQ(remainder.Capacity(), 899u);
    EXPECT_NE(remainder.Id(), small.Id());
    Fill(remainder, 5000);
    EXPECT_EQ(remainder.Size(), 899u);

    // Both views stay independent and intact.
    EXPECT_TRUE(Verify(mgr.Get(small.Id()), 9000));
    EXPECT_TRUE(Verify(mgr.Get(remainder.Id()), 5000));
    mgr.Free(remainder.Id());
    mgr.Free(small.Id());
}

TEST_F(IdArrayMgrTest, ClearAndReuse) {
    Mgr mgr;

    auto           arr = mgr.Allocate(32);
    const uint32_t id  = arr.Id();
    Fill(arr, 4242);
    mgr.Clear();

    EXPECT_THROW((void)mgr.Get(id), std::out_of_range);
    EXPECT_EQ(mgr.Get(0).Size(), 0u);

    auto again = mgr.Allocate(32);
    EXPECT_EQ(again.Capacity(), 32u);
    EXPECT_NE(again.Id(), 0u);
    Fill(again, 1);
    EXPECT_EQ(again.Size(), 32u);
    EXPECT_TRUE(Verify(mgr.Get(again.Id()), 1));
    mgr.Clear();
    EXPECT_EQ(mgr.Get(0).Size(), 0u);
}

TEST_F(IdArrayMgrTest, DesignAlias) {
    db::DesignIdArrayMgr mgr;
    auto                 arr = mgr.Allocate(3);
    EXPECT_EQ(arr.Capacity(), 3u);
    arr.PushBack(11);
    arr.PushBack(22);
    arr.PushBack(33);
    auto view = mgr.Get(arr.Id());
    EXPECT_EQ(view.Size(), 3u);
    EXPECT_TRUE(view[0] == 11 && view[1] == 22 && view[2] == 33);
    mgr.Free(arr.Id());
    EXPECT_THROW((void)mgr.Get(arr.Id()), std::out_of_range);
}

TEST_F(IdArrayMgrTest, PersistenceEmpty) {
    Mgr               mgr;
    const std::string bytes = SaveToBytes(mgr);

    Mgr loaded;
    EXPECT_TRUE(LoadFromBytes(loaded, bytes));
    CheckManagerEmpty(loaded);
    EXPECT_NE(loaded.Allocate(3).Id(), 0u);
}

TEST_F(IdArrayMgrTest, PersistenceRoundtrip) {
    struct Rec {
        uint32_t id;
        size_t   len;
        uint32_t seed;
    };

    const std::vector<size_t> lengths = {1, 16, 100, 1000};
    Mgr                       mgr;
    std::vector<Rec>          recs;
    for (size_t i = 0; i < lengths.size(); ++i) {
        const uint32_t seed = static_cast<uint32_t>(100 + i * 7);
        auto           arr  = mgr.Allocate(lengths[i]);
        EXPECT_NE(arr.Id(), 0u);
        Fill(arr, seed);
        recs.push_back({arr.Id(), lengths[i], seed});
    }

    // Free one array: its ID, freed state and length must survive the roundtrip.
    const uint32_t freed_id  = recs[1].id;
    const size_t   freed_len = recs[1].len;
    mgr.Free(freed_id);

    const std::string bytes = SaveToBytes(mgr);

    Mgr loaded;
    EXPECT_TRUE(LoadFromBytes(loaded, bytes));
    EXPECT_EQ(loaded.Get(0).Size(), 0u);
    for (size_t i = 0; i < recs.size(); ++i) {
        if (recs[i].id == freed_id) {
            continue;
        }
        auto view = loaded.Get(recs[i].id);
        EXPECT_EQ(view.Id(), recs[i].id);
        EXPECT_EQ(view.Capacity(), recs[i].len);
        EXPECT_EQ(view.Size(), recs[i].len);
        EXPECT_TRUE(Verify(view, recs[i].seed));
    }
    EXPECT_THROW((void)loaded.Get(freed_id), std::out_of_range);

    // Loaded free chunks are reusable with the same ID.
    auto reused = loaded.Allocate(freed_len);
    EXPECT_EQ(reused.Id(), freed_id);
    EXPECT_EQ(reused.Capacity(), freed_len);
    Fill(reused, 7777);
    EXPECT_EQ(reused.Size(), freed_len);
    EXPECT_TRUE(Verify(loaded.Get(reused.Id()), 7777));

    // A second Save/Load keeps every ID and value, including the reused one.
    const std::string bytes2 = SaveToBytes(loaded);
    Mgr               again;
    EXPECT_TRUE(LoadFromBytes(again, bytes2));
    for (const Rec& rec : recs) {
        auto view = again.Get(rec.id);
        EXPECT_EQ(view.Size(), rec.len);
        if (rec.id == freed_id) {
            EXPECT_TRUE(Verify(view, 7777));
        } else {
            EXPECT_TRUE(Verify(view, rec.seed));
        }
    }
}

TEST_F(IdArrayMgrTest, PersistenceSplitAfterLoad) {
    Mgr            mgr;
    auto           big    = mgr.Allocate(1000);
    const uint32_t big_id = big.Id();
    Fill(big, 5);
    mgr.Free(big_id);

    Mgr loaded;
    EXPECT_TRUE(LoadFromBytes(loaded, SaveToBytes(mgr)));

    // The loaded free chunk can be reused by splitting it.
    auto small = loaded.Allocate(100);
    EXPECT_EQ(small.Id(), big_id);
    EXPECT_EQ(small.Capacity(), 100u);
    Fill(small, 111);

    auto remainder = loaded.Allocate(899);  // 1000 - 100 - length word
    EXPECT_EQ(remainder.Capacity(), 899u);
    EXPECT_NE(remainder.Id(), small.Id());
    Fill(remainder, 222);

    EXPECT_TRUE(Verify(loaded.Get(small.Id()), 111));
    EXPECT_TRUE(Verify(loaded.Get(remainder.Id()), 222));

    // The split result is stable across another roundtrip.
    Mgr again;
    EXPECT_TRUE(LoadFromBytes(again, SaveToBytes(loaded)));
    EXPECT_TRUE(Verify(again.Get(small.Id()), 111));
    EXPECT_TRUE(Verify(again.Get(remainder.Id()), 222));
}

TEST_F(IdArrayMgrTest, PersistenceFailures) {
    Mgr  mgr;
    auto arr = mgr.Allocate(16);
    Fill(arr, 42);
    const std::string good = SaveToBytes(mgr);

    // Bad magic.
    {
        std::string bad = good;
        bad[0]          = static_cast<char>(bad[0] ^ 0xFF);
        Mgr m;
        EXPECT_FALSE(LoadFromBytes(m, bad));
        CheckManagerEmpty(m);
    }
    // Truncated payload.
    {
        std::string bad = good.substr(0, good.size() - 1);
        Mgr         m;
        EXPECT_FALSE(LoadFromBytes(m, bad));
        CheckManagerEmpty(m);
    }
    // Truncated header.
    {
        Mgr m;
        EXPECT_FALSE(LoadFromBytes(m, good.substr(0, 4)));
        CheckManagerEmpty(m);
    }
    // Wrong id width.
    {
        std::string bad;
        auto        put = [&bad](const auto& value) {
            bad.append(reinterpret_cast<const char*>(&value), sizeof(value));
        };
        put(uint32_t{0x49444152u});
        put(uint32_t{1u});
        put(uint32_t{8u});  // id width mismatch
        put(uint32_t{4u});
        put(uint64_t{1u});
        Mgr m;
        EXPECT_FALSE(LoadFromBytes(m, bad));
        CheckManagerEmpty(m);
    }
    // Wrong version.
    {
        std::string bad;
        auto        put = [&bad](const auto& value) {
            bad.append(reinterpret_cast<const char*>(&value), sizeof(value));
        };
        put(uint32_t{0x49444152u});
        put(uint32_t{2u});
        put(uint32_t{4u});
        put(uint32_t{4u});
        put(uint64_t{1u});
        Mgr m;
        EXPECT_FALSE(LoadFromBytes(m, bad));
        CheckManagerEmpty(m);
    }
    // Zero slot count.
    {
        std::string bad;
        auto        put = [&bad](const auto& value) {
            bad.append(reinterpret_cast<const char*>(&value), sizeof(value));
        };
        put(uint32_t{0x49444152u});
        put(uint32_t{1u});
        put(uint32_t{4u});
        put(uint32_t{4u});
        put(uint64_t{0u});
        Mgr m;
        EXPECT_FALSE(LoadFromBytes(m, bad));
        CheckManagerEmpty(m);
    }
}

}  // namespace
