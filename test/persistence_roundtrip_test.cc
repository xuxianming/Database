// End-to-end persistence test for design/module names backed by StringMgr.
//
// Uses the current working directory for the .db files (ctest runs in the
// build directory). Exits normally; ObjectTable owns live-slot destruction.

#include "DataModel/Design/Design.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"

#include <cstdio>
#include <cstdlib>

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::fprintf(stderr, "CHECK failed: %s\n  at %s:%d\n", #cond,        \
                         __FILE__, __LINE__);                                    \
            std::_Exit(1);                                                       \
        }                                                                        \
    } while (false)

int main() {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::Module;

    Design* design = DesignMemoryMgr::CreateDesign();
    CHECK(design != nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    Module* module = design->CreateModule("top");
    CHECK(module != nullptr);
    CHECK(module->GetName() == "top");
    const db::ObjectId module_id = module->GetObjectId();

    CHECK(DesignMemoryMgr::SaveAll());
    CHECK(DesignMemoryMgr::LoadAll("."));

    Design* loaded_design = Design::GetCurrentDesign();
    CHECK(loaded_design != nullptr);
    CHECK(loaded_design->GetStringMgr() != nullptr);
    CHECK(loaded_design->GetIdArrayMgr() != nullptr);

    Module* loaded_module = DesignMemoryMgr::GetModule(module_id);
    CHECK(loaded_module != nullptr);
    CHECK(loaded_module->GetName() == "top");
    CHECK(loaded_module->GetNameId() ==
          loaded_design->GetStringMgr()->Find("top").value_or(0));

    // Destroy the loaded objects through the table; normal exit must not crash.
    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyModule(module_id);
    DesignMemoryMgr::DestroyDesign(loaded_design->GetObjectId());

    std::printf("persistence_roundtrip_test: all checks passed\n");
    return 0;
}
