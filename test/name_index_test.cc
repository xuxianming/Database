// Standalone tests for the per-module NameId -> ObjectId index and the
// Module/Inst/HInst linked-list wiring (including a Save/Load rebuild).
//
// Build & run:
//   cmake --build <build> --target name_index_test && ./bin/name_index_test

#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/MemoryMgr.h"

#include <cstdio>
#include <cstdlib>
#include <vector>

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::fprintf(stderr, "CHECK failed: %s\n  at %s:%d\n", #cond,        \
                         __FILE__, __LINE__);                                    \
            std::exit(1);                                                        \
        }                                                                        \
    } while (false)

int main() {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::HInst;
    using db::Inst;
    using db::LibCell;
    using db::LibertyMemoryMgr;
    using db::Module;
    using db::ModuleHInstIterator;
    using db::ModuleInstIterator;

    LibertyMemoryMgr::EnsureManagers();

    Design* design = DesignMemoryMgr::CreateDesign();
    CHECK(design != nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    Module* top   = design->CreateModule("top");
    Module* other = design->CreateModule("other");
    CHECK(top != nullptr);
    CHECK(other != nullptr);

    LibCell* cell = LibertyMemoryMgr::CreateLibCell();
    CHECK(cell != nullptr);

    Inst* u0 = top->CreateInst(cell, "u0");
    Inst* u1 = top->CreateInst(cell, "u1");
    Inst* o0 = other->CreateInst(cell, "u0");
    CHECK(u0 != nullptr);
    CHECK(u1 != nullptr);
    CHECK(o0 != nullptr);

    // Per-module lookups stay independent.
    CHECK(top->FindInst("u0") == u0);
    CHECK(top->FindInst("u1") == u1);
    CHECK(other->FindInst("u0") == o0);
    CHECK(top->FindInst("missing") == nullptr);
    CHECK(other->FindInst("u1") == nullptr);

    {
        ModuleInstIterator      iter(top);
        std::vector<db::ObjectId> ids;
        while (iter.HasNext()) {
            Inst* inst = iter.Next();
            CHECK(inst != nullptr);
            ids.push_back(inst->GetObjectId());
        }
        CHECK(ids.size() == 2);
        CHECK((ids[0] == u1->GetObjectId() && ids[1] == u0->GetObjectId()) ||
              (ids[0] == u0->GetObjectId() && ids[1] == u1->GetObjectId()));
    }

    Module* child = design->CreateModule("child");
    CHECK(child != nullptr);
    HInst* h0 = top->CreateHInst(child, "h0");
    CHECK(h0 != nullptr);
    CHECK(top->FindHInst("h0") == h0);
    CHECK(top->FindHInst("missing") == nullptr);
    {
        ModuleHInstIterator       iter(top);
        std::vector<db::ObjectId> ids;
        while (iter.HasNext()) {
            HInst* hinst = iter.Next();
            CHECK(hinst != nullptr);
            ids.push_back(hinst->GetObjectId());
        }
        CHECK(ids.size() == 1);
        CHECK(ids[0] == h0->GetObjectId());
    }

    // Destroying an instance unlinks it and invalidates the module index.
    const db::ObjectId u0_id      = u0->GetObjectId();
    const db::ObjectId u1_id      = u1->GetObjectId();
    const db::ObjectId o0_id      = o0->GetObjectId();
    const db::ObjectId h0_id      = h0->GetObjectId();
    const db::ObjectId top_id     = top->GetObjectId();
    const db::ObjectId other_id   = other->GetObjectId();
    const db::ObjectId child_id   = child->GetObjectId();
    const db::ObjectId cell_id    = cell->GetObjectId();
    const db::ObjectId design_id  = design->GetObjectId();

    DesignMemoryMgr::DestroyInst(u0_id);
    CHECK(top->FindInst("u0") == nullptr);
    CHECK(top->FindInst("u1") == u1);
    {
        ModuleInstIterator iter(top);
        size_t             count = 0;
        while (iter.HasNext()) {
            CHECK(iter.Next() != nullptr);
            ++count;
        }
        CHECK(count == 1);
    }

    // Persistence: the linked lists are part of the raw object layout, so the
    // lazy index must rebuild from the restored lists after LoadAll.
    CHECK(DesignMemoryMgr::SaveAll());
    CHECK(DesignMemoryMgr::LoadAll("."));

    Design* loaded_design = Design::GetCurrentDesign();
    CHECK(loaded_design != nullptr);
    CHECK(loaded_design->GetObjectId() == design_id);
    CHECK(loaded_design->GetStringMgr() != nullptr);
    CHECK(loaded_design->GetIdArrayMgr() != nullptr);

    Module* loaded_top = DesignMemoryMgr::GetModule(top_id);
    CHECK(loaded_top != nullptr);
    CHECK(loaded_top->FindInst("u0") == nullptr);
    Inst* loaded_u1 = loaded_top->FindInst("u1");
    CHECK(loaded_u1 != nullptr);
    CHECK(loaded_u1->GetObjectId() == u1_id);

    Module* loaded_other = DesignMemoryMgr::GetModule(other_id);
    CHECK(loaded_other != nullptr);
    Inst* loaded_o0 = loaded_other->FindInst("u0");
    CHECK(loaded_o0 != nullptr);
    CHECK(loaded_o0->GetObjectId() == o0_id);

    HInst* loaded_h0 = loaded_top->FindHInst("h0");
    CHECK(loaded_h0 != nullptr);
    CHECK(loaded_h0->GetObjectId() == h0_id);
    CHECK(loaded_top->FindHInst("missing") == nullptr);

    {
        ModuleInstIterator iter(loaded_top);
        size_t             count = 0;
        while (iter.HasNext()) {
            CHECK(iter.Next() != nullptr);
            ++count;
        }
        CHECK(count == 1);
    }

    DesignMemoryMgr::DestroyInst(u1_id);
    DesignMemoryMgr::DestroyInst(o0_id);
    DesignMemoryMgr::DestroyHInst(h0_id);
    DesignMemoryMgr::DestroyModule(child_id);
    DesignMemoryMgr::DestroyModule(other_id);
    DesignMemoryMgr::DestroyModule(top_id);
    LibertyMemoryMgr::DestroyLibCell(cell_id);

    LibertyMemoryMgr liberty_mgr;
    liberty_mgr.Clear();

    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(loaded_design->GetObjectId());

    std::printf("name_index_test: all checks passed\n");
    return 0;
}
