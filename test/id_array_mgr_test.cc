// Standalone tests for db::IdArrayMgr / DynLenArrayTable.
//
// Build & run:
//   cmake --build <build> --target id_array_mgr_test && ./bin/id_array_mgr_test

#include "BasePlatfrom/IdArrayMgr.h"
#include "DataModel/Design/Type.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::fprintf(stderr, "CHECK failed: %s\n  at %s:%d\n", #cond,        \
                         __FILE__, __LINE__);                                    \
            std::exit(1);                                                        \
        }                                                                        \
    } while (false)

namespace {

using Mgr = db::IdArrayMgr<uint32_t, uint32_t, 15>;

template <typename Exception, typename Fn>
bool Throws(Fn&& fn) {
    try {
        fn();
    } catch (const Exception&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

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

void TestEmptyAndZeroId() {
    Mgr mgr;
    auto empty = mgr.Allocate(0);
    CHECK(empty.Size() == 0);
    CHECK(empty.Id() == 0);

    CHECK(mgr.Get(0).Size() == 0);
    CHECK(mgr.Get(0).Id() == 0);
    mgr.Free(0);  // no-op
}

void TestLengths() {
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
        CHECK(arr.Capacity() == len);
        CHECK(arr.Id() != 0);
        Fill(arr, static_cast<uint32_t>(i * 1000 + 1));
        CHECK(arr.Size() == len);
        recs.push_back({arr.Id(), len});
    }

    // Views still resolve to the same ids/data through Get().
    for (size_t i = 0; i < recs.size(); ++i) {
        auto view = mgr.Get(recs[i].id);
        CHECK(view.Id() == recs[i].id);
        CHECK(view.Size() == recs[i].len);
        CHECK(view.Capacity() == recs[i].len);
        CHECK(Verify(view, static_cast<uint32_t>(i * 1000 + 1)));
    }

    // Free -> Get must reject the id; double Free must throw.
    const uint32_t freed_id = recs[1].id;
    mgr.Free(freed_id);
    CHECK(Throws<std::out_of_range>([&] { (void)mgr.Get(freed_id); }));
    CHECK(Throws<std::runtime_error>([&] { mgr.Free(freed_id); }));

    // Out-of-range ids and oversized requests are rejected.
    CHECK(Throws<std::out_of_range>([&] { (void)mgr.Get(999999); }));
    CHECK(Throws<std::out_of_range>([&] { mgr.Free(999999); }));
    CHECK(Throws<std::runtime_error>([&] { (void)mgr.Allocate(65536); }));
}

void TestExactReuse() {
    Mgr mgr;

    auto first = mgr.Allocate(16);
    const uint32_t id = first.Id();
    Fill(first, 1111);
    mgr.Free(id);

    auto reused = mgr.Allocate(16);
    CHECK(reused.Id() == id);
    CHECK(reused.Capacity() == 16);
    Fill(reused, 2222);
    CHECK(reused.Size() == 16);
    CHECK(Verify(mgr.Get(id), 2222));
}

void TestSplit() {
    Mgr mgr;

    auto big      = mgr.Allocate(1000);
    const uint32_t big_id = big.Id();
    Fill(big, 7);
    mgr.Free(big_id);

    auto small = mgr.Allocate(100);
    CHECK(small.Id() == big_id);  // larger free block is split in place
    CHECK(small.Capacity() == 100);
    Fill(small, 9000);
    CHECK(small.Size() == 100);

    auto remainder = mgr.Allocate(899);  // 1000 - 100 - length word
    CHECK(remainder.Capacity() == 899);
    CHECK(remainder.Id() != small.Id());
    Fill(remainder, 5000);
    CHECK(remainder.Size() == 899);

    // Both views stay independent and intact.
    CHECK(Verify(mgr.Get(small.Id()), 9000));
    CHECK(Verify(mgr.Get(remainder.Id()), 5000));
    mgr.Free(remainder.Id());
    mgr.Free(small.Id());
}

void TestClearAndReuse() {
    Mgr mgr;

    auto arr        = mgr.Allocate(32);
    const uint32_t id = arr.Id();
    Fill(arr, 4242);
    mgr.Clear();

    CHECK(Throws<std::out_of_range>([&] { (void)mgr.Get(id); }));
    CHECK(mgr.Get(0).Size() == 0);

    auto again = mgr.Allocate(32);
    CHECK(again.Capacity() == 32);
    CHECK(again.Id() != 0);
    Fill(again, 1);
    CHECK(again.Size() == 32);
    CHECK(Verify(mgr.Get(again.Id()), 1));
    mgr.Clear();
    CHECK(mgr.Get(0).Size() == 0);
}

void TestDesignAlias() {
    db::DesignIdArrayMgr mgr;
    auto                 arr = mgr.Allocate(3);
    CHECK(arr.Capacity() == 3);
    arr.PushBack(11);
    arr.PushBack(22);
    arr.PushBack(33);
    auto view = mgr.Get(arr.Id());
    CHECK(view.Size() == 3);
    CHECK(view[0] == 11 && view[1] == 22 && view[2] == 33);
    mgr.Free(arr.Id());
    CHECK(Throws<std::out_of_range>([&] { (void)mgr.Get(arr.Id()); }));
}

std::string SaveToBytes(const Mgr& mgr) {
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    CHECK(mgr.Save(ss));
    return ss.str();
}

bool LoadFromBytes(Mgr& mgr, const std::string& bytes) {
    std::istringstream in(bytes, std::ios::in | std::ios::binary);
    return mgr.Load(in);
}

void CheckManagerEmpty(Mgr& mgr) {  // after a failed load
    CHECK(mgr.Get(0).Size() == 0);
    CHECK(Throws<std::out_of_range>([&] { (void)mgr.Get(1); }));
}

void TestPersistenceEmpty() {
    Mgr mgr;
    const std::string bytes = SaveToBytes(mgr);

    Mgr loaded;
    CHECK(LoadFromBytes(loaded, bytes));
    CheckManagerEmpty(loaded);
    CHECK(loaded.Allocate(3).Id() != 0);
}

void TestPersistenceRoundtrip() {
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
        CHECK(arr.Id() != 0);
        Fill(arr, seed);
        recs.push_back({arr.Id(), lengths[i], seed});
    }

    // Free one array: its ID, freed state and length must survive the roundtrip.
    const uint32_t freed_id  = recs[1].id;
    const size_t   freed_len = recs[1].len;
    mgr.Free(freed_id);

    const std::string bytes = SaveToBytes(mgr);

    Mgr loaded;
    CHECK(LoadFromBytes(loaded, bytes));
    CHECK(loaded.Get(0).Size() == 0);
    for (size_t i = 0; i < recs.size(); ++i) {
        if (recs[i].id == freed_id) {
            continue;
        }
        auto view = loaded.Get(recs[i].id);
        CHECK(view.Id() == recs[i].id);
        CHECK(view.Capacity() == recs[i].len);
        CHECK(view.Size() == recs[i].len);
        CHECK(Verify(view, recs[i].seed));
    }
    CHECK(Throws<std::out_of_range>([&] { (void)loaded.Get(freed_id); }));

    // Loaded free chunks are reusable with the same ID.
    auto reused = loaded.Allocate(freed_len);
    CHECK(reused.Id() == freed_id);
    CHECK(reused.Capacity() == freed_len);
    Fill(reused, 7777);
    CHECK(reused.Size() == freed_len);
    CHECK(Verify(loaded.Get(reused.Id()), 7777));

    // A second Save/Load keeps every ID and value, including the reused one.
    const std::string bytes2 = SaveToBytes(loaded);
    Mgr               again;
    CHECK(LoadFromBytes(again, bytes2));
    for (const Rec& rec : recs) {
        auto view = again.Get(rec.id);
        CHECK(view.Size() == rec.len);
        if (rec.id == freed_id) {
            CHECK(Verify(view, 7777));
        } else {
            CHECK(Verify(view, rec.seed));
        }
    }
}

void TestPersistenceSplitAfterLoad() {
    Mgr mgr;
    auto big = mgr.Allocate(1000);
    const uint32_t big_id = big.Id();
    Fill(big, 5);
    mgr.Free(big_id);

    Mgr loaded;
    CHECK(LoadFromBytes(loaded, SaveToBytes(mgr)));

    // The loaded free chunk can be reused by splitting it.
    auto small = loaded.Allocate(100);
    CHECK(small.Id() == big_id);
    CHECK(small.Capacity() == 100);
    Fill(small, 111);

    auto remainder = loaded.Allocate(899);  // 1000 - 100 - length word
    CHECK(remainder.Capacity() == 899);
    CHECK(remainder.Id() != small.Id());
    Fill(remainder, 222);

    CHECK(Verify(loaded.Get(small.Id()), 111));
    CHECK(Verify(loaded.Get(remainder.Id()), 222));

    // The split result is stable across another roundtrip.
    Mgr again;
    CHECK(LoadFromBytes(again, SaveToBytes(loaded)));
    CHECK(Verify(again.Get(small.Id()), 111));
    CHECK(Verify(again.Get(remainder.Id()), 222));
}

void TestPersistenceFailures() {
    Mgr mgr;
    auto arr = mgr.Allocate(16);
    Fill(arr, 42);
    const std::string good = SaveToBytes(mgr);

    // Bad magic.
    {
        std::string bad = good;
        bad[0]          = static_cast<char>(bad[0] ^ 0xFF);
        Mgr m;
        CHECK(!LoadFromBytes(m, bad));
        CheckManagerEmpty(m);
    }
    // Truncated payload.
    {
        std::string bad = good.substr(0, good.size() - 1);
        Mgr         m;
        CHECK(!LoadFromBytes(m, bad));
        CheckManagerEmpty(m);
    }
    // Truncated header.
    {
        Mgr m;
        CHECK(!LoadFromBytes(m, good.substr(0, 4)));
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
        CHECK(!LoadFromBytes(m, bad));
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
        CHECK(!LoadFromBytes(m, bad));
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
        CHECK(!LoadFromBytes(m, bad));
        CheckManagerEmpty(m);
    }
}

}  // namespace

int main() {
    TestEmptyAndZeroId();
    TestLengths();
    TestExactReuse();
    TestSplit();
    TestClearAndReuse();
    TestDesignAlias();
    TestPersistenceEmpty();
    TestPersistenceRoundtrip();
    TestPersistenceSplitAfterLoad();
    TestPersistenceFailures();
    std::printf("id_array_mgr_test: all checks passed\n");
    return 0;
}
