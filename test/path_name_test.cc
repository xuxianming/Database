// Standalone tests for on-demand hierarchical path names.
//
// Build & run:
//   cmake --build <build> --target path_name_test && ./bin/path_name_test

#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/MemoryMgr.h"

#include <cstdio>
#include <cstdlib>
#include <string>

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

    Design* design = DesignMemoryMgr::CreateDesign();
    CHECK(design != nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    Module* top   = design->CreateModule("top");
    Module* child = design->CreateModule("child");
    CHECK(top != nullptr);
    CHECK(child != nullptr);

    // No view HInst attached yet: module paths are just their local names.
    CHECK(top->PathName() == "top");
    CHECK(child->PathName() == "child");

    // A hierarchy instance of "child" under "top" gets a synthesized path.
    HInst* hinst = top->CreateHInst(child, "u0");
    CHECK(hinst != nullptr);
    CHECK(hinst->GetName() == "u0");
    CHECK(hinst->PathName() == "top/u0");
    CHECK(hinst->GetMasterDesign() == design);

    // An instance inside "top" is prefixed with the owner module path.
    LibCell* cell = LibertyMemoryMgr::CreateLibCell();
    CHECK(cell != nullptr);
    Inst* inst = top->CreateInst(cell, "i0");
    CHECK(inst != nullptr);
    CHECK(inst->GetName() == "i0");
    CHECK(inst->PathName() == "top/i0");

    // Virtual dispatch through Object must use the same override.
    db::Object* as_object = inst;
    CHECK(as_object->PathName() == "top/i0");

    DesignMemoryMgr::DestroyInst(inst->GetObjectId());
    LibertyMemoryMgr::DestroyLibCell(cell->GetObjectId());

    // Liberty managers are process-static; release them explicitly in tests.
    LibertyMemoryMgr liberty_mgr;
    liberty_mgr.Clear();

    DesignMemoryMgr::DestroyHInst(hinst->GetObjectId());
    DesignMemoryMgr::DestroyModule(child->GetObjectId());
    DesignMemoryMgr::DestroyModule(top->GetObjectId());

    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(design->GetObjectId());

    std::printf("path_name_test: all checks passed\n");
    return 0;
}
