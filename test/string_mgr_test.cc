// Standalone tests for db::StringMgr (no external test framework).
//
// Build & run:
//   g++ -std=c++20 -I/root/Database test/string_mgr_test.cc -o /tmp/opencode/string_mgr_test
//   /tmp/opencode/string_mgr_test

#include "BasePlatfrom/StringMgr.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
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

using Pool32 = db::StringMgr<uint32_t>;
using Pool64 = db::StringMgr<uint64_t>;

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

void TestConstruct() {
    Pool32 pool;
    CHECK(pool.Size() == 1);
    CHECK(pool.Count() == 0);
    CHECK(pool.Empty());
    CHECK(pool.GetString(0).empty());
    CHECK(pool.GetString(1).empty());
    CHECK(pool.Find("").value_or(0) == 1);
    CHECK(pool.Find("missing").has_value() == false);
}

void TestEmptyString() {
    Pool32 pool;
    CHECK(pool.AddString("") == 1);
    CHECK(pool.AddString(std::string_view("")) == 1);
    CHECK(pool.AddString(std::string("")) == 1);
    CHECK(pool.GetString(1).empty());
    CHECK(pool.Size() == 1);
    CHECK(pool.Count() == 0);
}

void TestDuplicates() {
    Pool32 pool;
    const auto a1 = pool.AddString("A");
    const auto a2 = pool.AddString("A");
    const auto a3 = pool.AddString(std::string("A"));
    CHECK(a1 == a2);
    CHECK(a2 == a3);
    CHECK(a1 == 2);

    const auto b = pool.AddString("B");
    CHECK(b == 3);
    CHECK(pool.Size() == 3);
    CHECK(pool.Count() == 2);
    CHECK(pool.Find("A").value() == a1);
    CHECK(pool.Find("B").value() == b);
    CHECK(!pool.Find("C").has_value());
}

void TestRoundtripLengths() {
    Pool32 pool;
    const std::vector<size_t> lengths = {1,   3,   4,   7,    8,    15,   16,
                                         127, 128, 255, 1024, 65535, 65536};
    std::vector<std::pair<uint32_t, std::string>> ids;
    for (size_t len : lengths) {
        const std::string s   = MakeString(len, static_cast<char>('a' + (len % 5)));
        const uint32_t    id  = pool.AddString(s);
        CHECK(id != 0);
        CHECK(pool.AddString(s) == id);
        CHECK(pool.GetString(id) == std::string_view(s));
        ids.emplace_back(id, s);
    }
    for (const auto& [id, s] : ids) {
        CHECK(pool.GetString(id) == std::string_view(s));
    }
    CHECK(pool.Count() == lengths.size());
}

void TestInvalidIds() {
    Pool32 pool;
    pool.AddString("x");
    CHECK(pool.GetString(0).empty());
    CHECK(pool.GetString(999).empty());
    CHECK(pool.GetString(static_cast<uint32_t>(-1)).empty());
}

void TestClear() {
    Pool32 pool;
    pool.AddString("A");
    pool.AddString("B");
    CHECK(pool.Size() == 3);

    pool.Clear();
    CHECK(pool.Size() == 1);
    CHECK(pool.Count() == 0);
    CHECK(pool.GetString(1).empty());
    CHECK(!pool.Find("A").has_value());
    CHECK(pool.AddString("A") == 2);
    CHECK(pool.GetString(2) == "A");
}

void TestViewStability() {
    Pool32 pool;
    const auto id   = pool.AddString("stable-name");
    const auto view = pool.GetString(id);
    CHECK(view == "stable-name");

    for (int i = 0; i < 10000; ++i) {
        pool.AddString("filler-" + std::to_string(i));
    }
    CHECK(view == "stable-name");
    CHECK(view.data() == pool.GetString(id).data());
    CHECK(pool.GetString(id) == "stable-name");
}

void TestSaveLoad() {
    Pool32 src;
    src.AddString("alpha");
    src.AddString("beta");
    src.AddString("gamma");
    src.AddString("alpha");  // duplicate, no new ID
    src.AddString(MakeString(5000, 'z'));

    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    CHECK(src.Save(ss));

    Pool32 dst;
    dst.AddString("pre-existing");  // must be wiped by Load
    CHECK(dst.Load(ss));
    CHECK(dst.Size() == src.Size());
    CHECK(dst.Count() == src.Count());
    for (uint32_t id = 1; id <= src.Size(); ++id) {
        CHECK(dst.GetString(id) == src.GetString(id));
    }
    CHECK(dst.Find("alpha").value() == src.Find("alpha").value());
    CHECK(dst.Find("beta").value() == src.Find("beta").value());
    CHECK(dst.AddString("alpha") == src.Find("alpha").value());
    CHECK(!dst.Find("pre-existing").has_value());
    CHECK(dst.GetString(static_cast<uint32_t>(src.Size())) == src.GetString(static_cast<uint32_t>(src.Size())));
}

void TestSaveLoadEmptyPool() {
    Pool32 src;
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    CHECK(src.Save(ss));

    Pool32 dst;
    CHECK(dst.Load(ss));
    CHECK(dst.Size() == 1);
    CHECK(dst.Count() == 0);
    CHECK(dst.GetString(1).empty());
}

void TestLoadPreservesDuplicates() {
    // A stream that contains "A" twice: Load must not deduplicate.
    std::string body;
    AppendRecord(body, "");
    AppendRecord(body, "A");
    AppendRecord(body, "A");
    const std::string raw = MakeRawStream(0x44425354u, 1u, sizeof(uint32_t), 3, body);

    std::istringstream in(raw, std::ios::in | std::ios::binary);
    Pool32             pool;
    CHECK(pool.Load(in));

    CHECK(pool.Size() == 3);
    CHECK(pool.Count() == 1);  // unique non-empty names ("A"); Load preserves the duplicate slot
    CHECK(pool.GetString(1).empty());
    CHECK(pool.GetString(2) == "A");
    CHECK(pool.GetString(3) == "A");
    CHECK(pool.Find("A").value() == 2);  // first index entry wins
    CHECK(pool.AddString("A") == 2);     // no new ID for an interned duplicate
}

void TestLoadFailures() {
    const uint32_t width = static_cast<uint32_t>(sizeof(uint32_t));

    // Bad magic.
    {
        std::string body;
        AppendRecord(body, "");
        std::istringstream in(
            MakeRawStream(0xDEADBEEFu, 1u, width, 1, body), std::ios::in | std::ios::binary);
        Pool32 pool;
        pool.AddString("keep-me");
        CHECK(!pool.Load(in));
        CHECK(pool.Size() == 1);
        CHECK(pool.GetString(1).empty());
        CHECK(!pool.Find("keep-me").has_value());
    }
    // Bad version.
    {
        std::string body;
        AppendRecord(body, "");
        std::istringstream in(
            MakeRawStream(0x44425354u, 3u, width, 1, body), std::ios::in | std::ios::binary);
        Pool32 pool;
        CHECK(!pool.Load(in));
        CHECK(pool.Size() == 1);
    }
    // Wrong id width.
    {
        std::string body;
        AppendRecord(body, "");
        std::istringstream in(
            MakeRawStream(0x44425354u, 1u, 8u, 1, body), std::ios::in | std::ios::binary);
        Pool32 pool;
        CHECK(!pool.Load(in));
        CHECK(pool.Size() == 1);
    }
    // Zero count.
    {
        std::istringstream in(MakeRawStream(0x44425354u, 1u, width, 0, std::string()),
                              std::ios::in | std::ios::binary);
        Pool32 pool;
        CHECK(!pool.Load(in));
        CHECK(pool.Size() == 1);
    }
    // Count beyond the sane cap.
    {
        std::istringstream in(MakeRawStream(0x44425354u, 1u, width, 100'000'001ull, std::string()),
                              std::ios::in | std::ios::binary);
        Pool32 pool;
        CHECK(!pool.Load(in));
        CHECK(pool.Size() == 1);
    }
    // Truncated stream (count says 2, only 1 record present).
    {
        std::string body;
        AppendRecord(body, "");
        std::istringstream in(MakeRawStream(0x44425354u, 1u, width, 2, body),
                              std::ios::in | std::ios::binary);
        Pool32 pool;
        CHECK(!pool.Load(in));
        CHECK(pool.Size() == 1);
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
        CHECK(!pool.Load(in));
        CHECK(pool.Size() == 1);
    }
    // First string not empty.
    {
        std::string body;
        AppendRecord(body, "not-empty");
        std::istringstream in(MakeRawStream(0x44425354u, 1u, width, 1, body),
                              std::ios::in | std::ios::binary);
        Pool32 pool;
        CHECK(!pool.Load(in));
        CHECK(pool.Size() == 1);
    }
}

void Test64BitIds() {
    Pool64 pool;
    CHECK(pool.AddString("") == 1);
    CHECK(pool.AddString("hello") == 2);
    CHECK(pool.GetString(2) == "hello");
    CHECK(pool.Find("hello").value() == 2);

    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    CHECK(pool.Save(ss));

    Pool64 dst;
    CHECK(dst.Load(ss));
    CHECK(dst.GetString(2) == "hello");
}

void TestArenaBlocks() {
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

    CHECK(id_a != 0 && id_b != 0 && id_c != 0 && id_d != 0);
    CHECK(pool.GetString(id_a) == std::string_view(near_block));
    CHECK(pool.GetString(id_b) == std::string_view(next_block));
    CHECK(pool.GetString(id_c) == std::string_view(exact_block));
    CHECK(pool.GetString(id_d) == std::string_view(over_block));

    // Long views stay valid while the arena grows and the index rehashes.
    for (int i = 0; i < 5000; ++i) {
        pool.AddString("filler-" + std::to_string(i));
    }
    CHECK(pool.GetString(id_a) == std::string_view(near_block));
    CHECK(pool.GetString(id_d) == std::string_view(over_block));

    // Long strings still deduplicate.
    CHECK(pool.AddString(exact_block) == id_c);
    CHECK(pool.AddString(over_block) == id_d);
}

void TestLargePool() {
    constexpr int kCount = 120000;
    Pool32        pool;

    std::vector<uint32_t>    spot_ids;
    std::vector<std::string> spot_strings;

    for (int i = 0; i < kCount; ++i) {
        std::string s  = "net_" + std::to_string(i);
        uint32_t    id = pool.AddString(s);
        CHECK(id != 0);
        if (i % 5000 == 0) {
            spot_ids.push_back(id);
            spot_strings.push_back(s);
        }
    }

    CHECK(pool.Count() == static_cast<size_t>(kCount));
    CHECK(pool.Size() == static_cast<size_t>(kCount) + 1);
    CHECK(pool.GetString(1).empty());
    CHECK(pool.GetString(2) == "net_0");

    for (size_t i = 0; i < spot_ids.size(); ++i) {
        CHECK(pool.GetString(spot_ids[i]) == std::string_view(spot_strings[i]));
        CHECK(pool.Find(spot_strings[i]).value() == spot_ids[i]);
    }

    // Duplicate lookups do not consume IDs or re-add storage.
    const uint32_t dup_id = pool.Find("net_777").value();
    CHECK(pool.AddString("net_777") == dup_id);
    CHECK(pool.Count() == static_cast<size_t>(kCount));

    // Save/Load preserves every ID and value.
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    CHECK(pool.Save(ss));

    Pool32 loaded;
    CHECK(loaded.Load(ss));
    CHECK(loaded.Count() == static_cast<size_t>(kCount));
    CHECK(loaded.Size() == static_cast<size_t>(kCount) + 1);
    for (size_t i = 0; i < spot_ids.size(); ++i) {
        CHECK(loaded.GetString(spot_ids[i]) == std::string_view(spot_strings[i]));
        CHECK(loaded.Find(spot_strings[i]).value() == spot_ids[i]);
    }
    CHECK(loaded.AddString("net_777") == dup_id);
    CHECK(loaded.GetString(1).empty());
}

void TestSeal() {
    Pool32 pool;
    const auto id = pool.AddString("A");
    pool.Seal();
    CHECK(pool.IsSealed());

    bool threw = false;
    try {
        pool.AddString("B");
    } catch (const std::logic_error&) {
        threw = true;
    }
    CHECK(threw);
    CHECK(pool.Find("A").value() == id);
    CHECK(pool.GetString(id) == "A");

    pool.Seal();  // idempotent
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    CHECK(pool.Save(ss));

    pool.Clear();
    CHECK(!pool.IsSealed());
    CHECK(pool.AddString("B") == 2);

    ss.seekg(0);
    Pool32 dst;
    dst.Seal();
    CHECK(dst.Load(ss));  // Load reopens the pool
    CHECK(!dst.IsSealed());
    CHECK(dst.GetString(2) == "A");
    CHECK(dst.AddString("C") == 3);
}

void TestRefCounting() {
    Pool32 pool;
    const uint32_t a = pool.AddString("A");
    CHECK(a == 2);
    CHECK(pool.AddString("A") == a);
    CHECK(pool.Count() == 1);
    CHECK(pool.Size() == 2);

    // One reference released: still live.
    pool.ReleaseString(a);
    CHECK(pool.Find("A").value() == a);
    CHECK(pool.Count() == 1);

    // Last reference released: Find skips it, GetString still readable.
    pool.ReleaseString(a);
    CHECK(!pool.Find("A").has_value());
    CHECK(pool.GetString(a) == "A");
    CHECK(pool.Count() == 0);
    CHECK(pool.Empty());
    CHECK(pool.Size() == 2);

    // Double release throws; invalid ids too; 0 is a no-op.
    CHECK(Throws<std::runtime_error>([&] { pool.ReleaseString(a); }));
    CHECK(Throws<std::out_of_range>([&] { pool.ReleaseString(999); }));
    pool.ReleaseString(0);

    // Resurrect the same ID; Count follows the refcount.
    CHECK(pool.AddString("A") == a);
    CHECK(pool.Find("A").value() == a);
    CHECK(pool.Count() == 1);
    CHECK(pool.AddString("A") == a);
    CHECK(pool.Count() == 1);

    // DestroyString is an alias for ReleaseString.
    pool.DestroyString(a);
    CHECK(pool.Find("A").value() == a);
    pool.DestroyString(a);
    CHECK(!pool.Find("A").has_value());
    CHECK(pool.GetString(a) == "A");

    // Save/Load v2 preserves dead views and their refcounts.
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    CHECK(pool.Save(ss));
    Pool32 loaded;
    CHECK(loaded.Load(ss));
    CHECK(loaded.Size() == pool.Size());
    CHECK(loaded.Count() == 0);
    CHECK(!loaded.Find("A").has_value());
    CHECK(loaded.GetString(a) == "A");
    CHECK(loaded.AddString("A") == a);
    CHECK(loaded.Find("A").value() == a);
    CHECK(loaded.Count() == 1);

    // Empty-string refs are counted but never affect unique Count.
    const size_t before = loaded.Count();
    CHECK(loaded.AddString("") == 1);
    CHECK(loaded.Count() == before);
    loaded.ReleaseString(1);
    CHECK(loaded.Find("").value() == 1);

    // DBST v1 files load with ref == 1 for every string.
    {
        std::string body;
        AppendRecord(body, "");
        AppendRecord(body, "legacy");
        std::istringstream in(MakeRawStream(0x44425354u, 1u, sizeof(uint32_t), 2, body),
                              std::ios::in | std::ios::binary);
        Pool32 legacy;
        CHECK(legacy.Load(in));
        CHECK(legacy.Find("legacy").value() == 2);
        CHECK(legacy.Count() == 1);
        legacy.ReleaseString(2);
        CHECK(!legacy.Find("legacy").has_value());
        CHECK(legacy.GetString(2) == "legacy");
        CHECK(legacy.Count() == 0);
    }
}

void TestPrintStats() {
    Pool32 pool;
    pool.AddString("abc");
    pool.AddString("de");
    pool.PrintStats();          // must print, not be an empty stub
    pool.PrintOptimizationInfo();
    CHECK(pool.Count() == 2);
}

}  // namespace

int main() {
    TestConstruct();
    TestEmptyString();
    TestDuplicates();
    TestRoundtripLengths();
    TestInvalidIds();
    TestClear();
    TestViewStability();
    TestSaveLoad();
    TestSaveLoadEmptyPool();
    TestLoadPreservesDuplicates();
    TestLoadFailures();
    Test64BitIds();
    TestArenaBlocks();
    TestLargePool();
    TestSeal();
    TestRefCounting();
    TestPrintStats();

    std::printf("string_mgr_test: all checks passed\n");
    return 0;
}
