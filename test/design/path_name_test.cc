// Tests for on-demand hierarchical path names.

#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/MemoryMgr.h"

#include <gtest/gtest.h>

#include <string>

class PathNameTest : public ::testing::Test {};

TEST_F(PathNameTest, HierarchicalPaths) {
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

    // No view HInst attached yet: module paths are just their local names.
    EXPECT_EQ(top->GetName(), "top");
    EXPECT_EQ(child->GetName(), "child");

    // A hierarchy occurrence of "child" under the design's top gets a path.
    HInst* hinst = design->CreateHInst(nullptr, child, "u0");
    ASSERT_NE(hinst, nullptr);
    EXPECT_EQ(hinst->GetName(), "u0");
    EXPECT_EQ(hinst->PathName(), "top/u0");
    EXPECT_EQ(hinst->GetModule(), top);
    EXPECT_EQ(hinst->GetDesign(), design);

    // Nested occurrences walk the HInst parent chain; no Module view needed.
    HInst* nested = design->CreateHInst(hinst, top, "u1");
    ASSERT_NE(nested, nullptr);
    EXPECT_EQ(nested->GetHInst(), hinst);
    EXPECT_EQ(nested->GetModule(), top);
    EXPECT_EQ(nested->PathName(), "top/u0/u1");

    // An instance inside "top" is prefixed with the owner module path.
    LibCell* cell = LibertyMemoryMgr::CreateLibCell();
    ASSERT_NE(cell, nullptr);
    Inst* inst = top->CreateInst(cell, "i0");
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(inst->GetName(), "i0");
    EXPECT_EQ(inst->PathName(), "top/i0");

    // Virtual dispatch through Object must use the same override.
    db::Object* as_object = inst;
    EXPECT_EQ(as_object->PathName(), "top/i0");

    DesignMemoryMgr::DestroyInst(inst->GetObjectId());
    LibertyMemoryMgr::DestroyLibCell(cell->GetObjectId());

    // Liberty managers are process-static; release them explicitly in tests.
    LibertyMemoryMgr liberty_mgr;
    liberty_mgr.Clear();

    DesignMemoryMgr::DestroyHInst(nested->GetObjectId());
    DesignMemoryMgr::DestroyHInst(hinst->GetObjectId());
    DesignMemoryMgr::DestroyModule(child->GetObjectId());
    DesignMemoryMgr::DestroyModule(top->GetObjectId());

    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(design->GetObjectId());
}
