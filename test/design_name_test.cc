// Standalone tests for design object names backed by StringMgr.
//
// Build & run:
//   g++ -std=c++20 -I/root/Database test/design_name_test.cc -o /tmp/opencode/design_name_test
//   /tmp/opencode/design_name_test

#include "DataModel/Design/Design.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"

#include <cstdio>
#include <cstdlib>
#include <string_view>

#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::fprintf(stderr, "CHECK failed: %s\n  at %s:%d\n", #cond,        \
                         __FILE__, __LINE__);                                    \
            std::fflush(stderr);                                                 \
            std::_Exit(1);                                                       \
        }                                                                        \
    } while (false)

int main() {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::Module;

    Design* design = DesignMemoryMgr::CreateDesign();
    CHECK(design != nullptr);
    CHECK(design->GetStringMgr() != nullptr);
    CHECK(design->GetIdArrayMgr() != nullptr);  // IdArrayMgr is dynamic-only (P0)

    Design::SetCurrentDesign(design->GetObjectId());

    // Same name must intern to the same ID and resolve back to the same view.
    Module* top1 = design->CreateModule("top");
    Module* top2 = design->CreateModule("top");
    CHECK(top1 != nullptr);
    CHECK(top2 != nullptr);
    CHECK(top1->GetName() == "top");
    CHECK(top2->GetName() == "top");
    CHECK(top1->GetNameId() == top2->GetNameId());
    CHECK(top1->GetNameId() == design->GetStringMgr()->Find("top").value_or(0));

    // Virtual dispatch still goes through the overload.
    db::Object* as_object = top1;
    CHECK(as_object->GetName() == "top");

    // Empty name maps to the pre-interned empty string (ID 1).
    Module* unnamed = design->CreateModule("");
    CHECK(unnamed != nullptr);
    CHECK(unnamed->GetName().empty());
    CHECK(unnamed->GetNameId() == 1);

    // Typed module creation also records its name.
    Module* adder = design->CreateModule(db::OperatorType::ADD, "adder");
    CHECK(adder != nullptr);
    CHECK(adder->GetName() == "adder");
    CHECK(adder->GetNameId() != top1->GetNameId());

    // Clone records the clone name.
    Module* clone = top1->Clone("top_clone");
    CHECK(clone != nullptr);
    CHECK(clone->GetName() == "top_clone");
    CHECK(clone->GetNameId() != top1->GetNameId());

    // Inst/HInst GetName paths are covered by guards; inst_create_test covers a
    // zero-port Inst through the design's IdArrayMgr.

    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(design->GetObjectId());

    std::printf("design_name_test: all checks passed\n");
    return 0;
}
