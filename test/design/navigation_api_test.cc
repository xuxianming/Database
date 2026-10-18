// Tests for the unified hierarchy navigation API:
// GetDesign / GetModule / GetHInst.

#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/MemoryMgr.h"

#include <gtest/gtest.h>

class NavigationApiTest : public ::testing::Test {};

TEST_F(NavigationApiTest, TopLevelInstAndHInstNavigation) {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::HInst;
    using db::Inst;
    using db::LibCell;
    using db::LibertyMemoryMgr;
    using db::Module;

    Design* design = DesignMemoryMgr::CreateDesign();
    ASSERT_NE(design, nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    Module* top   = design->CreateModule("top");
    Module* child = design->CreateModule("child");
    ASSERT_NE(top, nullptr);
    ASSERT_NE(child, nullptr);
    EXPECT_EQ(top, design->GetTopModule());
    EXPECT_EQ(top->GetDesign(), design);
    EXPECT_EQ(top->GetDesign(), design);

    LibCell* cell = LibertyMemoryMgr::CreateLibCell();
    ASSERT_NE(cell, nullptr);

    // A topmodule-direct Inst is owned by the topmodule and has no HInst.
    Inst* top_inst = top->CreateInst(cell, "i0");
    ASSERT_NE(top_inst, nullptr);
    EXPECT_EQ(top_inst->GetModule(), top);
    EXPECT_EQ(top_inst->GetHInst(), nullptr);
    EXPECT_EQ(top_inst->GetDesign(), design);
    EXPECT_EQ(top_inst->GetDesign(), design);

    // An Inst defined in a non-top module still navigates to the topmodule,
    // while its owner module remains the defining module.
    Inst* child_inst = child->CreateInst(cell, "i1");
    ASSERT_NE(child_inst, nullptr);
    EXPECT_EQ(child_inst->GetModule(), top);
    EXPECT_EQ(child_inst->GetHInst(), nullptr);
    EXPECT_EQ(child_inst->GetDesign(), design);

    // Top-level HInst occurrence of "child" under "top".
    HInst* hinst = design->CreateHInst(nullptr, child, "u0");
    ASSERT_NE(hinst, nullptr);
    EXPECT_EQ(hinst->GetModule(), top);
    EXPECT_EQ(hinst->GetHInst(), nullptr);
    EXPECT_EQ(hinst->GetDesign(), design);
    EXPECT_EQ(hinst->GetDesign(), design);
    EXPECT_EQ(hinst->GetTemplateModule(), child);

    // Nested HInst keeps the same topmodule and design; GetHInst is the upper HInst.
    HInst* nested = design->CreateHInst(hinst, child, "u1");
    ASSERT_NE(nested, nullptr);
    EXPECT_EQ(nested->GetHInst(), hinst);
    EXPECT_EQ(nested->GetModule(), top);
    EXPECT_EQ(nested->GetDesign(), design);

    // Cloning a module retains its original design.
    Module* clone = top->Clone("top_clone");
    ASSERT_NE(clone, nullptr);
    EXPECT_EQ(clone->GetDesign(), design);
    EXPECT_EQ(clone->GetDesign(), design);

    const db::ObjectId top_inst_id   = top_inst->GetObjectId();
    const db::ObjectId child_inst_id = child_inst->GetObjectId();
    const db::ObjectId nested_id     = nested->GetObjectId();
    const db::ObjectId hinst_id      = hinst->GetObjectId();
    const db::ObjectId clone_id      = clone->GetObjectId();
    const db::ObjectId child_id      = child->GetObjectId();
    const db::ObjectId top_id        = top->GetObjectId();
    const db::ObjectId cell_id       = cell->GetObjectId();
    const db::ObjectId design_id     = design->GetObjectId();

    DesignMemoryMgr::DestroyInst(top_inst_id);
    DesignMemoryMgr::DestroyInst(child_inst_id);
    DesignMemoryMgr::DestroyHInst(nested_id);
    DesignMemoryMgr::DestroyHInst(hinst_id);
    DesignMemoryMgr::DestroyModule(clone_id);
    DesignMemoryMgr::DestroyModule(child_id);
    DesignMemoryMgr::DestroyModule(top_id);
    LibertyMemoryMgr::DestroyLibCell(cell_id);

    LibertyMemoryMgr liberty_mgr;
    liberty_mgr.Clear();

    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(design_id);
}
