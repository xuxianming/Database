// Tests for design object names backed by StringMgr.

#include "DataModel/Design/Design.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"

#include <gtest/gtest.h>

#include <string_view>

class DesignNameTest : public ::testing::Test {};

TEST_F(DesignNameTest, ModuleNamesInternAndResolve) {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::Module;

    Design* design = DesignMemoryMgr::CreateDesign();
    ASSERT_NE(design, nullptr);
    ASSERT_NE(design->GetStringMgr(), nullptr);
    ASSERT_NE(design->GetIdArrayMgr(), nullptr);  // IdArrayMgr is dynamic-only (P0)

    Design::SetCurrentDesign(design->GetObjectId());

    // Same name must intern to the same ID and resolve back to the same view.
    Module* top1 = design->CreateModule("top");
    Module* top2 = design->CreateModule("top");
    ASSERT_NE(top1, nullptr);
    ASSERT_NE(top2, nullptr);
    EXPECT_EQ(top1->GetName(), "top");
    EXPECT_EQ(top2->GetName(), "top");
    EXPECT_EQ(top1->GetNameId(), top2->GetNameId());
    EXPECT_EQ(top1->GetNameId(), design->GetStringMgr()->Find("top").value_or(0));

    // Virtual dispatch still goes through the overload.
    db::Object* as_object = top1;
    EXPECT_EQ(as_object->GetName(), "top");

    // Empty name maps to the pre-interned empty string (ID 1).
    Module* unnamed = design->CreateModule("");
    ASSERT_NE(unnamed, nullptr);
    EXPECT_TRUE(unnamed->GetName().empty());
    EXPECT_EQ(unnamed->GetNameId(), 1u);

    // Typed module creation also records its name.
    Module* adder = design->CreateModule(db::OperatorType::ADD, "adder");
    ASSERT_NE(adder, nullptr);
    EXPECT_EQ(adder->GetName(), "adder");
    EXPECT_NE(adder->GetNameId(), top1->GetNameId());

    // Clone records the clone name.
    Module* clone = top1->Clone("top_clone");
    ASSERT_NE(clone, nullptr);
    EXPECT_EQ(clone->GetName(), "top_clone");
    EXPECT_NE(clone->GetNameId(), top1->GetNameId());

    // Inst/HInst GetName paths are covered separately; inst_create_test covers a
    // zero-port Inst through the design's IdArrayMgr.

    Design::SetCurrentDesign(db::kInvalidId);

    // Destroy every module created by this test before tearing down the design.
    DesignMemoryMgr::DestroyModule(clone->GetObjectId());
    DesignMemoryMgr::DestroyModule(adder->GetObjectId());
    DesignMemoryMgr::DestroyModule(unnamed->GetObjectId());
    DesignMemoryMgr::DestroyModule(top2->GetObjectId());
    DesignMemoryMgr::DestroyModule(top1->GetObjectId());

    DesignMemoryMgr::DestroyDesign(design->GetObjectId());
}
