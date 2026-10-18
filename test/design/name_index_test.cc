// Tests for the per-module NameId -> ObjectId index and the
// Design/HInst hierarchy tree wiring (including a Save/Load rebuild).

#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/MemoryMgr.h"

#include <gtest/gtest.h>

#include <vector>

class NameIndexTest : public ::testing::Test {};

TEST_F(NameIndexTest, PerModuleLookupsAndPersistence) {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::DesignTopHInstIterator;
    using db::HInst;
    using db::HInstHInstIterator;
    using db::Inst;
    using db::LibCell;
    using db::LibertyMemoryMgr;
    using db::Module;
    using db::ModuleInstIterator;

    LibertyMemoryMgr::EnsureManagers();

    Design* design = DesignMemoryMgr::CreateDesign();
    ASSERT_NE(design, nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    Module* top   = design->CreateModule("top");
    Module* other = design->CreateModule("other");
    ASSERT_NE(top, nullptr);
    ASSERT_NE(other, nullptr);

    LibCell* cell = LibertyMemoryMgr::CreateLibCell();
    ASSERT_NE(cell, nullptr);

    Inst* u0 = top->CreateInst(cell, "u0");
    Inst* u1 = top->CreateInst(cell, "u1");
    Inst* o0 = other->CreateInst(cell, "u0");
    ASSERT_NE(u0, nullptr);
    ASSERT_NE(u1, nullptr);
    ASSERT_NE(o0, nullptr);

    // Per-module lookups stay independent.
    EXPECT_EQ(top->FindInst("u0"), u0);
    EXPECT_EQ(top->FindInst("u1"), u1);
    EXPECT_EQ(other->FindInst("u0"), o0);
    EXPECT_EQ(top->FindInst("missing"), nullptr);
    EXPECT_EQ(other->FindInst("u1"), nullptr);

    {
        ModuleInstIterator        iter(top);
        std::vector<db::ObjectId> ids;
        while (iter.HasNext()) {
            Inst* inst = iter.Next();
            ASSERT_NE(inst, nullptr);
            ids.push_back(inst->GetObjectId());
        }
        EXPECT_EQ(ids.size(), 2u);
        EXPECT_TRUE((ids[0] == u1->GetObjectId() && ids[1] == u0->GetObjectId()) ||
                    (ids[0] == u0->GetObjectId() && ids[1] == u1->GetObjectId()));
    }

    // Hierarchy occurrences belong to the Design/HInst tree, not to Module.
    Module* child = design->CreateModule("child");
    ASSERT_NE(child, nullptr);

    HInst* h0 = design->CreateHInst(nullptr, child, "h0");
    ASSERT_NE(h0, nullptr);
    EXPECT_EQ(h0->GetModule(), top);
    EXPECT_EQ(h0->GetTemplateModule(), child);
    EXPECT_EQ(h0->GetHInst(), nullptr);
    EXPECT_EQ(h0->PathName(), "top/h0");

    HInst* h1 = design->CreateHInst(h0, other, "h1");
    ASSERT_NE(h1, nullptr);
    EXPECT_EQ(h1->GetModule(), top);
    EXPECT_EQ(h1->GetTemplateModule(), other);
    EXPECT_EQ(h1->GetHInst(), h0);
    EXPECT_EQ(h1->PathName(), "top/h0/h1");

    {
        DesignTopHInstIterator    iter(design);
        std::vector<db::ObjectId> ids;
        while (iter.HasNext()) {
            HInst* hinst = iter.Next();
            ASSERT_NE(hinst, nullptr);
            ids.push_back(hinst->GetObjectId());
        }
        EXPECT_EQ(ids.size(), 1u);
        EXPECT_EQ(ids[0], h0->GetObjectId());
    }
    {
        HInstHInstIterator        iter(h0);
        std::vector<db::ObjectId> ids;
        while (iter.HasNext()) {
            HInst* hinst = iter.Next();
            ASSERT_NE(hinst, nullptr);
            ids.push_back(hinst->GetObjectId());
        }
        EXPECT_EQ(ids.size(), 1u);
        EXPECT_EQ(ids[0], h1->GetObjectId());
    }
    {
        HInstHInstIterator iter(h1);
        EXPECT_FALSE(iter.HasNext());
        EXPECT_EQ(iter.Next(), nullptr);
    }

    // Destroying an instance unlinks it and invalidates the module index.
    const db::ObjectId u0_id     = u0->GetObjectId();
    const db::ObjectId u1_id     = u1->GetObjectId();
    const db::ObjectId o0_id     = o0->GetObjectId();
    const db::ObjectId h0_id     = h0->GetObjectId();
    const db::ObjectId h1_id     = h1->GetObjectId();
    const db::ObjectId top_id    = top->GetObjectId();
    const db::ObjectId other_id  = other->GetObjectId();
    const db::ObjectId child_id  = child->GetObjectId();
    const db::ObjectId cell_id   = cell->GetObjectId();
    const db::ObjectId design_id = design->GetObjectId();

    DesignMemoryMgr::DestroyInst(u0_id);
    EXPECT_EQ(top->FindInst("u0"), nullptr);
    EXPECT_EQ(top->FindInst("u1"), u1);
    {
        ModuleInstIterator iter(top);
        size_t             count = 0;
        while (iter.HasNext()) {
            EXPECT_NE(iter.Next(), nullptr);
            ++count;
        }
        EXPECT_EQ(count, 1u);
    }

    // Persistence: the linked lists are part of the raw object layout, so both
    // the lazy Module index and the HInst tree must rebuild after LoadAll.
    ASSERT_TRUE(DesignMemoryMgr::SaveAll());
    ASSERT_TRUE(DesignMemoryMgr::LoadAll("."));

    Design* loaded_design = Design::GetCurrentDesign();
    ASSERT_NE(loaded_design, nullptr);
    EXPECT_EQ(loaded_design->GetObjectId(), design_id);
    EXPECT_NE(loaded_design->GetStringMgr(), nullptr);
    EXPECT_NE(loaded_design->GetIdArrayMgr(), nullptr);

    Module* loaded_top = DesignMemoryMgr::GetModule(top_id);
    ASSERT_NE(loaded_top, nullptr);
    EXPECT_EQ(loaded_top->FindInst("u0"), nullptr);
    Inst* loaded_u1 = loaded_top->FindInst("u1");
    ASSERT_NE(loaded_u1, nullptr);
    EXPECT_EQ(loaded_u1->GetObjectId(), u1_id);
    EXPECT_EQ(loaded_u1->GetModule(), loaded_top);
    EXPECT_EQ(loaded_u1->GetHInst(), nullptr);
    EXPECT_EQ(loaded_u1->GetDesign(), loaded_design);

    Module* loaded_other = DesignMemoryMgr::GetModule(other_id);
    ASSERT_NE(loaded_other, nullptr);
    Inst* loaded_o0 = loaded_other->FindInst("u0");
    ASSERT_NE(loaded_o0, nullptr);
    EXPECT_EQ(loaded_o0->GetObjectId(), o0_id);

    HInst* loaded_h0 = DesignMemoryMgr::GetHInst(h0_id);
    ASSERT_NE(loaded_h0, nullptr);
    EXPECT_EQ(loaded_h0->GetModule(), loaded_top);
    EXPECT_EQ(loaded_h0->GetTemplateModule(), DesignMemoryMgr::GetModule(child_id));
    EXPECT_EQ(loaded_h0->GetHInst(), nullptr);
    EXPECT_EQ(loaded_h0->GetDesign(), loaded_design);
    EXPECT_EQ(loaded_h0->PathName(), "top/h0");

    HInst* loaded_h1 = DesignMemoryMgr::GetHInst(h1_id);
    ASSERT_NE(loaded_h1, nullptr);
    EXPECT_EQ(loaded_h1->GetModule(), loaded_top);
    EXPECT_EQ(loaded_h1->GetHInst(), loaded_h0);
    EXPECT_EQ(loaded_h1->PathName(), "top/h0/h1");

    {
        DesignTopHInstIterator iter(loaded_design);
        ASSERT_TRUE(iter.HasNext());
        EXPECT_EQ(iter.Next(), loaded_h0);
        EXPECT_FALSE(iter.HasNext());
    }
    {
        HInstHInstIterator iter(loaded_h0);
        ASSERT_TRUE(iter.HasNext());
        EXPECT_EQ(iter.Next(), loaded_h1);
        EXPECT_FALSE(iter.HasNext());
    }

    {
        ModuleInstIterator iter(loaded_top);
        size_t             count = 0;
        while (iter.HasNext()) {
            EXPECT_NE(iter.Next(), nullptr);
            ++count;
        }
        EXPECT_EQ(count, 1u);
    }

    DesignMemoryMgr::DestroyInst(u1_id);
    DesignMemoryMgr::DestroyInst(o0_id);
    DesignMemoryMgr::DestroyHInst(h1_id);
    DesignMemoryMgr::DestroyHInst(h0_id);
    DesignMemoryMgr::DestroyModule(child_id);
    DesignMemoryMgr::DestroyModule(other_id);
    DesignMemoryMgr::DestroyModule(top_id);
    LibertyMemoryMgr::DestroyLibCell(cell_id);

    LibertyMemoryMgr liberty_mgr;
    liberty_mgr.Clear();

    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(loaded_design->GetObjectId());
}
