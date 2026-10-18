// Tests for db::StringMgr.

#include "BasePlatfrom/StringMgr.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using Pool32 = db::StringMgr<uint32_t>;
using Pool64 = db::StringMgr<uint64_t>;

class StringMgrTest : public ::testing::Test {};

std::string MakeString(size_t len, char seed = 'a') {
    std::string s;
    s.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        s.push_back(static_cast<char>(seed + static_cast<char>(i % 26)));
    }
    return s;
}

// Builds a raw "DBST" stream; `body` is the concatenated length+bytes records.
std::string MakeRawStream(uint32_t magic, uint32_t version, uint32_t width,
                          uint64_t count, const std::string& body) {
    std::string out;
    auto append_pod = [&out](const auto& v) {
        out.append(reinterpret_cast<const char*>(&v), sizeof(v));
    };
    append_pod(magic);
    append_pod(version);
    append_pod(width);
    append_pod(count);
    out += body;
    return out;
}

void AppendRecord(std::string& body, std::string_view s) {
    const uint32_t len = static_cast<uint32_t>(s.size());
    body.append(reinterpret_cast<const char*>(&len), sizeof(len));
    body.append(s.data(), s.size());
}

TEST_F(StringMgrTest, Construct) {
    Pool32 pool;
    EXPECT_EQ(pool.Size(), 1u);
    EXPECT_EQ(pool.Count(), 0u);
    EXPECT_TRUE(pool.Empty());
    EXPECT_TRUE(pool.GetString(0).empty());
    EXPECT_TRUE(pool.GetString(1).empty());
    EXPECT_EQ(pool.Find("").value_or(0), 1u);
    EXPECT_FALSE(pool.Find("missing").has_value());
}

TEST_F(StringMgrTest, EmptyString) {
    Pool32 pool;
    EXPECT_EQ(pool.AddString(""), 1u);
    EXPECT_EQ(pool.AddString(std::string_view("")), 1u);
    EXPECT_EQ(pool.AddString(std::string("")), 1u);
    EXPECT_TRUE(pool.GetString(1).empty());
    EXPECT_EQ(pool.Size(), 1u);
    EXPECT_EQ(pool.Count(), 0u);
}

TEST_F(StringMgrTest, Duplicates) {
    Pool32 pool;
    const auto a1 = pool.AddString("A");
    const auto a2 = pool.AddString("A");
    const auto a3 = pool.AddString(std::string("A"));
    EXPECT_EQ(a1, a2);
    EXPECT_EQ(a2, a3);
    EXPECT_EQ(a1, 2u);

    const auto b = pool.AddString("B");
    EXPECT_EQ(b, 3u);
    EXPECT_EQ(pool.Size(), 3u);
    EXPECT_EQ(pool.Count(), 2u);
    EXPECT_EQ(pool.Find("A").value(), a1);
    EXPECT_EQ(pool.Find("B").value(), b);
    EXPECT_FALSE(pool.Find("C").has_value());
}

TEST_F(StringMgrTest, RoundtripLengths) {
    Pool32 pool;
    const std::vector<size_t> lengths = {1,   3,   4,   7,    8,    15,   16,
                                         127, 128, 255, 1024, 65535, 65536};
    std::vector<std::pair<uint32_t, std::string>> ids;
    for (size_t len : lengths) {
        const std::string s   = MakeString(len, static_cast<char>('a' + (len % 5)));
        const uint32_t    id  = pool.AddString(s);
        EXPECT_NE(id, 0u);
        EXPECT_EQ(pool.AddString(s), id);
        EXPECT_EQ(pool.GetString(id), std::string_view(s));
        ids.emplace_back(id, s);
    }
    for (const auto& [id, s] : ids) {
        EXPECT_EQ(pool.GetString(id), std::string_view(s));
    }
    EXPECT_EQ(pool.Count(), lengths.size());
}

TEST_F(StringMgrTest, InvalidIds) {
    Pool32 pool;
    pool.AddString("x");
    EXPECT_TRUE(pool.GetString(0).empty());
    EXPECT_TRUE(pool.GetString(999).empty());
    EXPECT_TRUE(pool.GetString(static_cast<uint32_t>(-1)).empty());
}

TEST_F(StringMgrTest, Clear) {
    Pool32 pool;
    pool.AddString("A");
    pool.AddString("B");
    EXPECT_EQ(pool.Size(), 3u);

    pool.Clear();
    EXPECT_EQ(pool.Size(), 1u);
    EXPECT_EQ(pool.Count(), 0u);
    EXPECT_TRUE(pool.GetString(1).empty());
    EXPECT_FALSE(pool.Find("A").has_value());
    EXPECT_EQ(pool.AddString("A"), 2u);
    EXPECT_EQ(pool.GetString(2), "A");
}

TEST_F(StringMgrTest, ViewStability) {
    Pool32 pool;
    const auto id   = pool.AddString("stable-name");
    const auto view = pool.GetString(id);
    EXPECT_EQ(view, "stable-name");

    for (int i = 0; i < 10000; ++i) {
        pool.AddString("filler-" + std::to_string(i));
    }
    EXPECT_EQ(view, "stable-name");
    EXPECT_EQ(view.data(), pool.GetString(id).data());
    EXPECT_EQ(pool.GetString(id), "stable-name");
}

TEST_F(StringMgrTest, SaveLoad) {
    Pool32 src;
    src.AddString("alpha");
    src.AddString("beta");
    src.AddString("gamma");
    src.AddString("alpha");  // duplicate, no new ID
    src.AddString(MakeString(5000, 'z'));

    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    ASSERT_TRUE(src.Save(ss));

    Pool32 dst;
    dst.AddString("pre-existing");  // must be wiped by Load
    ASSERT_TRUE(dst.Load(ss));
    EXPECT_EQ(dst.Size(), src.Size());
    EXPECT_EQ(dst.Count(), src.Count());
    for (uint32_t id = 1; id <= src.Size(); ++id) {
        EXPECT_EQ(dst.GetString(id), src.GetString(id));
    }
    const auto src_alpha = src.Find("alpha");
    const auto dst_alpha = dst.Find("alpha");
    ASSERT_TRUE(src_alpha.has_value());
    ASSERT_TRUE(dst_alpha.has_value());
    EXPECT_EQ(*dst_alpha, *src_alpha);

    const auto src_beta = src.Find("beta");
    const auto dst_beta = dst.Find("beta");
    ASSERT_TRUE(src_beta.has_value());
    ASSERT_TRUE(dst_beta.has_value());
    EXPECT_EQ(*dst_beta, *src_beta);

    EXPECT_EQ(dst.AddString("alpha"), *src_alpha);
    EXPECT_FALSE(dst.Find("pre-existing").has_value());
    EXPECT_EQ(dst.GetString(static_cast<uint32_t>(src.Size())),
              src.GetString(static_cast<uint32_t>(src.Size())));
}

TEST_F(StringMgrTest, SaveLoadEmptyPool) {
    Pool32 src;
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    ASSERT_TRUE(src.Save(ss));

    Pool32 dst;
    ASSERT_TRUE(dst.Load(ss));
    EXPECT_EQ(dst.Size(), 1u);
    EXPECT_EQ(dst.Count(), 0u);
    EXPECT_TRUE(dst.GetString(1).empty());
}

TEST_F(StringMgrTest, LoadPreservesDuplicates) {
    // A stream that contains "A" twice: Load must not deduplicate.
    std::string body;
    AppendRecord(body, "");
    AppendRecord(body, "A");
    AppendRecord(body, "A");
    const std::string raw = MakeRawStream(0x44425354u, 1u, sizeof(uint32_t), 3, body);

    std::istringstream in(raw, std::ios::in | std::ios::binary);
    Pool32             pool;
    ASSERT_TRUE(pool.Load(in));

    EXPECT_EQ(pool.Size(), 3u);
    EXPECT_EQ(pool.Count(), 1u);  // unique non-empty names ("A"); Load preserves the duplicate slot
    EXPECT_TRUE(pool.GetString(1).empty());
    EXPECT_EQ(pool.GetString(2), "A");
    EXPECT_EQ(pool.GetString(3), "A");
    EXPECT_EQ(pool.Find("A").value(), 2u);  // first index entry wins
    EXPECT_EQ(pool.AddString("A"), 2u);     // no new ID for an interned duplicate
}

TEST_F(StringMgrTest, LoadFailures) {
    const uint32_t width = static_cast<uint32_t>(sizeof(uint32_t));

    // Bad magic.
    {
        std::string body;
        AppendRecord(body, "");
        std::istringstream in(
            MakeRawStream(0xDEADBEEFu, 1u, width, 1, body), std::ios::in | std::ios::binary);
        Pool32 pool;
        pool.AddString("keep-me");
        EXPECT_FALSE(pool.Load(in));
        EXPECT_EQ(pool.Size(), 1u);
        EXPECT_TRUE(pool.GetString(1).empty());
        EXPECT_FALSE(pool.Find("keep-me").has_value());
    }
    // Bad version.
    {
        std::string body;
        AppendRecord(body, "");
        std::istringstream in(
            MakeRawStream(0x44425354u, 3u, width, 1, body), std::ios::in | std::ios::binary);
        Pool32 pool;
        EXPECT_FALSE(pool.Load(in));
        EXPECT_EQ(pool.Size(), 1u);
    }
    // Wrong id width.
    {
        std::string body;
        AppendRecord(body, "");
        std::istringstream in(
            MakeRawStream(0x44425354u, 1u, 8u, 1, body), std::ios::in | std::ios::binary);
        Pool32 pool;
        EXPECT_FALSE(pool.Load(in));
        EXPECT_EQ(pool.Size(), 1u);
    }
    // Zero count.
    {
        std::istringstream in(MakeRawStream(0x44425354u, 1u, width, 0, std::string()),
                              std::ios::in | std::ios::binary);
        Pool32 pool;
        EXPECT_FALSE(pool.Load(in));
        EXPECT_EQ(pool.Size(), 1u);
    }
    // Count beyond the sane cap.
    {
        std::istringstream in(MakeRawStream(0x44425354u, 1u, width, 100'000'001ull, std::string()),
                              std::ios::in | std::ios::binary);
        Pool32 pool;
        EXPECT_FALSE(pool.Load(in));
        EXPECT_EQ(pool.Size(), 1u);
    }
    // Truncated stream (count says 2, only 1 record present).
    {
        std::string body;
        AppendRecord(body, "");
        std::istringstream in(MakeRawStream(0x44425354u, 1u, width, 2, body),
                              std::ios::in | std::ios::binary);
        Pool32 pool;
        EXPECT_FALSE(pool.Load(in));
        EXPECT_EQ(pool.Size(), 1u);
    }
    // Truncated payload (valid header, payload cut short).
    {
        std::string body;
        AppendRecord(body, "");
        AppendRecord(body, std::string(100, 'x'));
        std::string raw = MakeRawStream(0x44425354u, 1u, width, 2, body);
        raw.resize(raw.size() - 10);
        std::istringstream in(raw, std::ios::in | std::ios::binary);
        Pool32 pool;
        pool.AddString("keep-me");
        EXPECT_FALSE(pool.Load(in));
        EXPECT_EQ(pool.Size(), 1u);
    }
    // First string not empty.
    {
        std::string body;
        AppendRecord(body, "not-empty");
        std::istringstream in(MakeRawStream(0x44425354u, 1u, width, 1, body),
                              std::ios::in | std::ios::binary);
        Pool32 pool;
        EXPECT_FALSE(pool.Load(in));
        EXPECT_EQ(pool.Size(), 1u);
    }
}

TEST_F(StringMgrTest, Ids64Bit) {
    Pool64 pool;
    EXPECT_EQ(pool.AddString(""), 1u);
    EXPECT_EQ(pool.AddString("hello"), 2u);
    EXPECT_EQ(pool.GetString(2), "hello");
    EXPECT_EQ(pool.Find("hello").value(), 2u);

    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    ASSERT_TRUE(pool.Save(ss));

    Pool64 dst;
    ASSERT_TRUE(dst.Load(ss));
    EXPECT_EQ(dst.GetString(2), "hello");
}

TEST_F(StringMgrTest, ArenaBlocks) {
    Pool32 pool;

    // 60000 bytes leaves ~5.5 KiB in the 64 KiB block; 10000 forces a new block.
    const std::string near_block(60000, 'a');
    const std::string next_block(10000, 'b');
    const std::string exact_block(65536, 'c');
    const std::string over_block(70000, 'd');

    const uint32_t id_a = pool.AddString(near_block);
    const uint32_t id_b = pool.AddString(next_block);
    const uint32_t id_c = pool.AddString(exact_block);
    const uint32_t id_d = pool.AddString(over_block);

    EXPECT_NE(id_a, 0u);
    EXPECT_NE(id_b, 0u);
    EXPECT_NE(id_c, 0u);
    EXPECT_NE(id_d, 0u);
    EXPECT_EQ(pool.GetString(id_a), std::string_view(near_block));
    EXPECT_EQ(pool.GetString(id_b), std::string_view(next_block));
    EXPECT_EQ(pool.GetString(id_c), std::string_view(exact_block));
    EXPECT_EQ(pool.GetString(id_d), std::string_view(over_block));

    // Long views stay valid while the arena grows and the index rehashes.
    for (int i = 0; i < 5000; ++i) {
        pool.AddString("filler-" + std::to_string(i));
    }
    EXPECT_EQ(pool.GetString(id_a), std::string_view(near_block));
    EXPECT_EQ(pool.GetString(id_d), std::string_view(over_block));

    // Long strings still deduplicate.
    EXPECT_EQ(pool.AddString(exact_block), id_c);
    EXPECT_EQ(pool.AddString(over_block), id_d);
}

TEST_F(StringMgrTest, LargePool120k) {
    constexpr int kCount = 120000;
    Pool32        pool;

    std::vector<uint32_t>    spot_ids;
    std::vector<std::string> spot_strings;

    for (int i = 0; i < kCount; ++i) {
        std::string s  = "net_" + std::to_string(i);
        uint32_t    id = pool.AddString(s);
        EXPECT_NE(id, 0u);
        if (i % 5000 == 0) {
            spot_ids.push_back(id);
            spot_strings.push_back(s);
        }
    }

    EXPECT_EQ(pool.Count(), static_cast<size_t>(kCount));
    EXPECT_EQ(pool.Size(), static_cast<size_t>(kCount) + 1);
    EXPECT_TRUE(pool.GetString(1).empty());
    EXPECT_EQ(pool.GetString(2), "net_0");

    for (size_t i = 0; i < spot_ids.size(); ++i) {
        EXPECT_EQ(pool.GetString(spot_ids[i]), std::string_view(spot_strings[i]));
        EXPECT_EQ(pool.Find(spot_strings[i]).value(), spot_ids[i]);
    }

    // Duplicate lookups do not consume IDs or re-add storage.
    const uint32_t dup_id = pool.Find("net_777").value();
    EXPECT_EQ(pool.AddString("net_777"), dup_id);
    EXPECT_EQ(pool.Count(), static_cast<size_t>(kCount));

    // Save/Load preserves every ID and value.
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    ASSERT_TRUE(pool.Save(ss));

    Pool32 loaded;
    ASSERT_TRUE(loaded.Load(ss));
    EXPECT_EQ(loaded.Count(), static_cast<size_t>(kCount));
    EXPECT_EQ(loaded.Size(), static_cast<size_t>(kCount) + 1);
    for (size_t i = 0; i < spot_ids.size(); ++i) {
        EXPECT_EQ(loaded.GetString(spot_ids[i]), std::string_view(spot_strings[i]));
        EXPECT_EQ(loaded.Find(spot_strings[i]).value(), spot_ids[i]);
    }
    EXPECT_EQ(loaded.AddString("net_777"), dup_id);
    EXPECT_TRUE(loaded.GetString(1).empty());
}

TEST_F(StringMgrTest, Seal) {
    Pool32 pool;
    const auto id = pool.AddString("A");
    pool.Seal();
    ASSERT_TRUE(pool.IsSealed());

    EXPECT_THROW(pool.AddString("B"), std::logic_error);
    EXPECT_EQ(pool.Find("A").value(), id);
    EXPECT_EQ(pool.GetString(id), "A");

    pool.Seal();  // idempotent
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    ASSERT_TRUE(pool.Save(ss));

    pool.Clear();
    EXPECT_FALSE(pool.IsSealed());
    EXPECT_EQ(pool.AddString("B"), 2u);

    ss.seekg(0);
    Pool32 dst;
    dst.Seal();
    ASSERT_TRUE(dst.Load(ss));  // Load reopens the pool
    EXPECT_FALSE(dst.IsSealed());
    EXPECT_EQ(dst.GetString(2), "A");
    EXPECT_EQ(dst.AddString("C"), 3u);
}

TEST_F(StringMgrTest, RefCounting) {
    Pool32 pool;
    const uint32_t a = pool.AddString("A");
    EXPECT_EQ(a, 2u);
    EXPECT_EQ(pool.AddString("A"), a);
    EXPECT_EQ(pool.Count(), 1u);
    EXPECT_EQ(pool.Size(), 2u);

    // One reference released: still live.
    pool.ReleaseString(a);
    EXPECT_EQ(pool.Find("A").value(), a);
    EXPECT_EQ(pool.Count(), 1u);

    // Last reference released: Find skips it, GetString still readable.
    pool.ReleaseString(a);
    EXPECT_FALSE(pool.Find("A").has_value());
    EXPECT_EQ(pool.GetString(a), "A");
    EXPECT_EQ(pool.Count(), 0u);
    EXPECT_TRUE(pool.Empty());
    EXPECT_EQ(pool.Size(), 2u);

    // Double release throws; invalid ids too; 0 is a no-op.
    EXPECT_THROW(pool.ReleaseString(a), std::runtime_error);
    EXPECT_THROW(pool.ReleaseString(999), std::out_of_range);
    pool.ReleaseString(0);

    // Resurrect the same ID; Count follows the refcount.
    EXPECT_EQ(pool.AddString("A"), a);
    EXPECT_EQ(pool.Find("A").value(), a);
    EXPECT_EQ(pool.Count(), 1u);
    EXPECT_EQ(pool.AddString("A"), a);
    EXPECT_EQ(pool.Count(), 1u);

    // DestroyString is an alias for ReleaseString.
    pool.DestroyString(a);
    EXPECT_EQ(pool.Find("A").value(), a);
    pool.DestroyString(a);
    EXPECT_FALSE(pool.Find("A").has_value());
    EXPECT_EQ(pool.GetString(a), "A");

    // Save/Load v2 preserves dead views and their refcounts.
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    ASSERT_TRUE(pool.Save(ss));
    Pool32 loaded;
    ASSERT_TRUE(loaded.Load(ss));
    EXPECT_EQ(loaded.Size(), pool.Size());
    EXPECT_EQ(loaded.Count(), 0u);
    EXPECT_FALSE(loaded.Find("A").has_value());
    EXPECT_EQ(loaded.GetString(a), "A");
    EXPECT_EQ(loaded.AddString("A"), a);
    EXPECT_EQ(loaded.Find("A").value(), a);
    EXPECT_EQ(loaded.Count(), 1u);

    // Empty-string refs are counted but never affect unique Count.
    const size_t before = loaded.Count();
    EXPECT_EQ(loaded.AddString(""), 1u);
    EXPECT_EQ(loaded.Count(), before);
    loaded.ReleaseString(1);
    EXPECT_EQ(loaded.Find("").value(), 1u);

    // DBST v1 files load with ref == 1 for every string.
    {
        std::string body;
        AppendRecord(body, "");
        AppendRecord(body, "legacy");
        std::istringstream in(MakeRawStream(0x44425354u, 1u, sizeof(uint32_t), 2, body),
                              std::ios::in | std::ios::binary);
        Pool32 legacy;
        ASSERT_TRUE(legacy.Load(in));
        EXPECT_EQ(legacy.Find("legacy").value(), 2u);
        EXPECT_EQ(legacy.Count(), 1u);
        legacy.ReleaseString(2);
        EXPECT_FALSE(legacy.Find("legacy").has_value());
        EXPECT_EQ(legacy.GetString(2), "legacy");
        EXPECT_EQ(legacy.Count(), 0u);
    }
}

TEST_F(StringMgrTest, PrintStats) {
    Pool32 pool;
    pool.AddString("abc");
    pool.AddString("de");
    pool.PrintStats();          // must print, not be an empty stub
    pool.PrintOptimizationInfo();
    EXPECT_EQ(pool.Count(), 2u);
}

}  // namespace
