#include "DataModel/Design/Design.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Liberty/MemoryMgr.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

namespace {

// One process now runs all suites. Clean global object tables and generated
// snapshots after every test so cases remain order-independent.
class DbTestCleanup : public ::testing::EmptyTestEventListener {
public:
    void OnTestEnd(const ::testing::TestInfo&) override {
        db::DesignMemoryMgr::Clear();

        db::LibertyMemoryMgr liberty_mgr;
        liberty_mgr.Clear();

        std::error_code ec;
        std::filesystem::remove("metadata.bin", ec);
        std::filesystem::remove("strings.db", ec);
        std::filesystem::remove("id_arrays.db", ec);
        for (int i = 0; i < 32; ++i) {
            std::filesystem::remove(std::to_string(i) + "_7bits.db", ec);
        }
    }
};

}  // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::UnitTest::GetInstance()->listeners().Append(new DbTestCleanup());
    return RUN_ALL_TESTS();
}
