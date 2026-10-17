// Standalone test: Module::CreateInst on a zero-port LibCell.
//
// Build & run:
//   cmake --build <build> --target inst_create_test && ./bin/inst_create_test

#include "DataModel/Design/Design.h"
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
    using db::Inst;
    using db::LibCell;
    using db::LibertyMemoryMgr;
    using db::Module;

    Design* design = DesignMemoryMgr::CreateDesign();
    CHECK(design != nullptr);
    CHECK(design->GetStringMgr() != nullptr);
    CHECK(design->GetIdArrayMgr() != nullptr);

    Design::SetCurrentDesign(design->GetObjectId());

    Module* module = design->CreateModule("top");
    CHECK(module != nullptr);
    CHECK(module->GetName() == "top");

    // A fresh cell has no ports: GetPortArray() must yield an empty view
    // (m_ports defaults to kInvalidId) and CreateInst must still succeed.
    LibCell* cell = LibertyMemoryMgr::CreateLibCell();
    CHECK(cell != nullptr);
    CHECK(cell->PortCount() == 0);
    CHECK(cell->GetPortArray().Size() == 0);

    Inst* inst = module->CreateInst(cell, "u0");
    CHECK(inst != nullptr);
    CHECK(inst->GetName() == "u0");
    CHECK(inst->GetModule() == module);
    CHECK(inst->GetMasterDesign() == design);
    CHECK(inst->PinCount() == 0);
    CHECK(design->GetIdArrayMgr() != nullptr);

    DesignMemoryMgr::DestroyInst(inst->GetObjectId());
    DesignMemoryMgr::DestroyModule(module->GetObjectId());
    LibertyMemoryMgr::DestroyLibCell(cell->GetObjectId());

    // Liberty managers are process-static; release them explicitly in tests.
    LibertyMemoryMgr liberty_mgr;
    liberty_mgr.Clear();

    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(design->GetObjectId());

    std::printf("inst_create_test: all checks passed\n");
    return 0;
}
