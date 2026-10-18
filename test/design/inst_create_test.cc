// Test: Module::CreateInst on a zero-port LibCell.

#include "DataModel/Design/Design.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/MemoryMgr.h"

#include <gtest/gtest.h>

class InstCreateTest : public ::testing::Test {};

TEST_F(InstCreateTest, ZeroPortLibCell) {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::Inst;
    using db::LibCell;
    using db::LibertyMemoryMgr;
    using db::Module;

    Design* design = DesignMemoryMgr::CreateDesign();
    ASSERT_NE(design, nullptr);
    ASSERT_NE(design->GetStringMgr(), nullptr);
    ASSERT_NE(design->GetIdArrayMgr(), nullptr);

    Design::SetCurrentDesign(design->GetObjectId());

    Module* module = design->CreateModule("top");
    ASSERT_NE(module, nullptr);
    EXPECT_EQ(module->GetName(), "top");

    // A fresh cell has no ports: GetPortArray() must yield an empty view
    // (m_ports defaults to kInvalidId) and CreateInst must still succeed.
    LibCell* cell = LibertyMemoryMgr::CreateLibCell();
    ASSERT_NE(cell, nullptr);
    EXPECT_EQ(cell->PortCount(), 0u);
    EXPECT_EQ(cell->GetPortArray().Size(), 0u);

    Inst* inst = module->CreateInst(cell, "u0");
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->GetName(), "u0");
    EXPECT_EQ(inst->GetModule(), module);
    EXPECT_EQ(inst->GetMasterDesign(), design);
    EXPECT_EQ(inst->PinCount(), 0u);
    EXPECT_NE(design->GetIdArrayMgr(), nullptr);

    DesignMemoryMgr::DestroyInst(inst->GetObjectId());
    DesignMemoryMgr::DestroyModule(module->GetObjectId());
    LibertyMemoryMgr::DestroyLibCell(cell->GetObjectId());

    // Liberty managers are process-static; release them explicitly in tests.
    LibertyMemoryMgr liberty_mgr;
    liberty_mgr.Clear();

    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(design->GetObjectId());
}
