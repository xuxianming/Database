// End-to-end persistence test for design/module names backed by StringMgr.
//
// Uses the current working directory for the .db files (ctest runs in the
// build directory). ObjectTable owns live-slot destruction.

#include "DataModel/Design/Design.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"

#include <gtest/gtest.h>

class PersistenceRoundtripTest : public ::testing::Test {};

TEST_F(PersistenceRoundtripTest, ModuleNameSurvivesSaveLoad) {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::Module;

    Design* design = DesignMemoryMgr::CreateDesign();
    ASSERT_NE(design, nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    Module* module = design->CreateModule("top");
    ASSERT_NE(module, nullptr);
    EXPECT_EQ(module->GetName(), "top");
    const db::ObjectId module_id = module->GetObjectId();

    ASSERT_TRUE(DesignMemoryMgr::SaveAll());
    ASSERT_TRUE(DesignMemoryMgr::LoadAll("."));

    Design* loaded_design = Design::GetCurrentDesign();
    ASSERT_NE(loaded_design, nullptr);
    EXPECT_NE(loaded_design->GetStringMgr(), nullptr);
    EXPECT_NE(loaded_design->GetIdArrayMgr(), nullptr);

    Module* loaded_module = DesignMemoryMgr::GetModule(module_id);
    ASSERT_NE(loaded_module, nullptr);
    EXPECT_EQ(loaded_module->GetName(), "top");
    EXPECT_EQ(loaded_module->GetNameId(),
              loaded_design->GetStringMgr()->Find("top").value_or(0));

    // Destroy the loaded objects through the table; normal exit must not crash.
    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyModule(module_id);
    DesignMemoryMgr::DestroyDesign(loaded_design->GetObjectId());
}
