// ECO test: rename + refcounted name release across Inst/HInst/Module,
// plus a Save/Load refcount roundtrip.

#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/MemoryMgr.h"

#include <gtest/gtest.h>

class EcoTest : public ::testing::Test {};

TEST_F(EcoTest, RenameReleaseAndPersistence) {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::DesignStringMgr;
    using db::HInst;
    using db::Inst;
    using db::LibCell;
    using db::LibertyMemoryMgr;
    using db::Module;

    LibertyMemoryMgr::EnsureManagers();

    Design* design = DesignMemoryMgr::CreateDesign();
    ASSERT_NE(design, nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    DesignStringMgr* names = design->GetStringMgr();
    ASSERT_NE(names, nullptr);

    Module* top   = design->CreateModule("top");
    Module* child = design->CreateModule("child");
    ASSERT_NE(top, nullptr);
    ASSERT_NE(child, nullptr);

    LibCell* cell = LibertyMemoryMgr::CreateLibCell();
    ASSERT_NE(cell, nullptr);

    // ---- Inst rename -----------------------------------------------------
    Inst* inst = top->CreateInst(cell, "u0");
    ASSERT_NE(inst, nullptr);
    EXPECT_EQ(top->FindInst("u0"), inst);
    EXPECT_TRUE(names->Find("u0").has_value());

    ASSERT_TRUE(inst->Rename("u1"));
    EXPECT_EQ(inst->GetName(), "u1");
    EXPECT_EQ(inst->PathName(), "top/u1");
    EXPECT_EQ(top->FindInst("u0"), nullptr);
    EXPECT_EQ(top->FindInst("u1"), inst);
    EXPECT_FALSE(names->Find("u0").has_value());
    EXPECT_TRUE(names->Find("u1").has_value());

    // ---- HInst rename ----------------------------------------------------
    HInst* hinst = top->CreateHInst(child, "h0");
    ASSERT_NE(hinst, nullptr);
    EXPECT_EQ(hinst->PathName(), "top/h0");

    ASSERT_TRUE(hinst->Rename("h1"));
    EXPECT_EQ(hinst->GetName(), "h1");
    EXPECT_EQ(hinst->PathName(), "top/h1");
    EXPECT_EQ(top->FindHInst("h0"), nullptr);
    EXPECT_EQ(top->FindHInst("h1"), hinst);
    EXPECT_FALSE(names->Find("h0").has_value());
    EXPECT_TRUE(names->Find("h1").has_value());

    // ---- Module rename keeps paths correct -------------------------------
    ASSERT_TRUE(top->Rename("top2"));
    EXPECT_EQ(top->GetName(), "top2");
    EXPECT_EQ(top->PathName(), "top2");
    EXPECT_EQ(inst->PathName(), "top2/u1");
    EXPECT_EQ(hinst->PathName(), "top2/h1");
    EXPECT_EQ(top->FindInst("u1"), inst);
    EXPECT_EQ(top->FindHInst("h1"), hinst);
    EXPECT_FALSE(names->Find("top").has_value());
    EXPECT_TRUE(names->Find("top2").has_value());

    const db::ObjectId inst_id  = inst->GetObjectId();
    const db::ObjectId hinst_id = hinst->GetObjectId();
    const db::ObjectId child_id = child->GetObjectId();
    const db::ObjectId top_id   = top->GetObjectId();
    const db::ObjectId cell_id  = cell->GetObjectId();

    // ---- Destroy releases the names -------------------------------------
    DesignMemoryMgr::DestroyInst(inst_id);
    EXPECT_FALSE(names->Find("u1").has_value());
    DesignMemoryMgr::DestroyHInst(hinst_id);
    EXPECT_FALSE(names->Find("h1").has_value());
    DesignMemoryMgr::DestroyModule(child_id);
    EXPECT_FALSE(names->Find("child").has_value());
    DesignMemoryMgr::DestroyModule(top_id);
    EXPECT_FALSE(names->Find("top2").has_value());

    // ---- Save/Load refcount roundtrip -----------------------------------
    Module* pm = design->CreateModule("pm");
    ASSERT_NE(pm, nullptr);
    Inst* pi = pm->CreateInst(cell, "pi");
    ASSERT_NE(pi, nullptr);
    ASSERT_TRUE(pi->Rename("pi2"));
    EXPECT_EQ(pm->FindInst("pi"), nullptr);
    EXPECT_EQ(pm->FindInst("pi2"), pi);

    const db::ObjectId design_id = design->GetObjectId();
    const db::ObjectId pm_id     = pm->GetObjectId();
    const db::ObjectId pi_id     = pi->GetObjectId();

    ASSERT_TRUE(DesignMemoryMgr::SaveAll());
    ASSERT_TRUE(DesignMemoryMgr::LoadAll("."));

    Design* loaded = Design::GetCurrentDesign();
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->GetObjectId(), design_id);
    DesignStringMgr* loaded_names = loaded->GetStringMgr();
    ASSERT_NE(loaded_names, nullptr);

    Module* loaded_pm = DesignMemoryMgr::GetModule(pm_id);
    ASSERT_NE(loaded_pm, nullptr);
    EXPECT_EQ(loaded_pm->FindInst("pi"), nullptr);
    Inst* loaded_pi = loaded_pm->FindInst("pi2");
    ASSERT_NE(loaded_pi, nullptr);
    EXPECT_EQ(loaded_pi->GetObjectId(), pi_id);
    EXPECT_EQ(loaded_pi->GetName(), "pi2");
    EXPECT_EQ(loaded_pi->PathName(), "pm/pi2");

    DesignMemoryMgr::DestroyInst(pi_id);
    EXPECT_FALSE(loaded_names->Find("pi2").has_value());
    DesignMemoryMgr::DestroyModule(pm_id);
    EXPECT_FALSE(loaded_names->Find("pm").has_value());
    EXPECT_FALSE(loaded_names->Find("pi").has_value());

    LibertyMemoryMgr::DestroyLibCell(cell_id);

    LibertyMemoryMgr liberty_mgr;
    liberty_mgr.Clear();

    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(design_id);
}
