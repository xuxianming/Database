// Standalone ECO test: rename + refcounted name release across Inst/HInst/Module,
// plus a Save/Load refcount roundtrip.
//
// Build & run:
//   cmake --build <build> --target eco_test && ./bin/eco_test

#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/MemoryMgr.h"

#include <cstdio>
#include <cstdlib>

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
    using db::DesignStringMgr;
    using db::HInst;
    using db::Inst;
    using db::LibCell;
    using db::LibertyMemoryMgr;
    using db::Module;

    LibertyMemoryMgr::EnsureManagers();

    Design* design = DesignMemoryMgr::CreateDesign();
    CHECK(design != nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    DesignStringMgr* names = design->GetStringMgr();
    CHECK(names != nullptr);

    Module* top   = design->CreateModule("top");
    Module* child = design->CreateModule("child");
    CHECK(top != nullptr);
    CHECK(child != nullptr);

    LibCell* cell = LibertyMemoryMgr::CreateLibCell();
    CHECK(cell != nullptr);

    // ---- Inst rename -----------------------------------------------------
    Inst* inst = top->CreateInst(cell, "u0");
    CHECK(inst != nullptr);
    CHECK(top->FindInst("u0") == inst);
    CHECK(names->Find("u0").has_value());

    CHECK(inst->Rename("u1"));
    CHECK(inst->GetName() == "u1");
    CHECK(inst->PathName() == "top/u1");
    CHECK(top->FindInst("u0") == nullptr);
    CHECK(top->FindInst("u1") == inst);
    CHECK(!names->Find("u0").has_value());
    CHECK(names->Find("u1").has_value());

    // ---- HInst rename ----------------------------------------------------
    HInst* hinst = top->CreateHInst(child, "h0");
    CHECK(hinst != nullptr);
    CHECK(hinst->PathName() == "top/h0");

    CHECK(hinst->Rename("h1"));
    CHECK(hinst->GetName() == "h1");
    CHECK(hinst->PathName() == "top/h1");
    CHECK(top->FindHInst("h0") == nullptr);
    CHECK(top->FindHInst("h1") == hinst);
    CHECK(!names->Find("h0").has_value());
    CHECK(names->Find("h1").has_value());

    // ---- Module rename keeps paths correct -------------------------------
    CHECK(top->Rename("top2"));
    CHECK(top->GetName() == "top2");
    CHECK(top->PathName() == "top2");
    CHECK(inst->PathName() == "top2/u1");
    CHECK(hinst->PathName() == "top2/h1");
    CHECK(top->FindInst("u1") == inst);
    CHECK(top->FindHInst("h1") == hinst);
    CHECK(!names->Find("top").has_value());
    CHECK(names->Find("top2").has_value());

    const db::ObjectId inst_id  = inst->GetObjectId();
    const db::ObjectId hinst_id = hinst->GetObjectId();
    const db::ObjectId child_id = child->GetObjectId();
    const db::ObjectId top_id   = top->GetObjectId();
    const db::ObjectId cell_id  = cell->GetObjectId();

    // ---- Destroy releases the names -------------------------------------
    DesignMemoryMgr::DestroyInst(inst_id);
    CHECK(!names->Find("u1").has_value());
    DesignMemoryMgr::DestroyHInst(hinst_id);
    CHECK(!names->Find("h1").has_value());
    DesignMemoryMgr::DestroyModule(child_id);
    CHECK(!names->Find("child").has_value());
    DesignMemoryMgr::DestroyModule(top_id);
    CHECK(!names->Find("top2").has_value());

    // ---- Save/Load refcount roundtrip -----------------------------------
    Module* pm = design->CreateModule("pm");
    CHECK(pm != nullptr);
    Inst* pi = pm->CreateInst(cell, "pi");
    CHECK(pi != nullptr);
    CHECK(pi->Rename("pi2"));
    CHECK(pm->FindInst("pi") == nullptr);
    CHECK(pm->FindInst("pi2") == pi);

    const db::ObjectId design_id = design->GetObjectId();
    const db::ObjectId pm_id     = pm->GetObjectId();
    const db::ObjectId pi_id     = pi->GetObjectId();

    CHECK(DesignMemoryMgr::SaveAll());
    CHECK(DesignMemoryMgr::LoadAll("."));

    Design* loaded = Design::GetCurrentDesign();
    CHECK(loaded != nullptr);
    CHECK(loaded->GetObjectId() == design_id);
    DesignStringMgr* loaded_names = loaded->GetStringMgr();
    CHECK(loaded_names != nullptr);

    Module* loaded_pm = DesignMemoryMgr::GetModule(pm_id);
    CHECK(loaded_pm != nullptr);
    CHECK(loaded_pm->FindInst("pi") == nullptr);
    Inst* loaded_pi = loaded_pm->FindInst("pi2");
    CHECK(loaded_pi != nullptr);
    CHECK(loaded_pi->GetObjectId() == pi_id);
    CHECK(loaded_pi->GetName() == "pi2");
    CHECK(loaded_pi->PathName() == "pm/pi2");

    DesignMemoryMgr::DestroyInst(pi_id);
    CHECK(!loaded_names->Find("pi2").has_value());
    DesignMemoryMgr::DestroyModule(pm_id);
    CHECK(!loaded_names->Find("pm").has_value());
    CHECK(!loaded_names->Find("pi").has_value());

    LibertyMemoryMgr::DestroyLibCell(cell_id);

    LibertyMemoryMgr liberty_mgr;
    liberty_mgr.Clear();

    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(design_id);

    std::printf("eco_test: all checks passed\n");
    return 0;
}
