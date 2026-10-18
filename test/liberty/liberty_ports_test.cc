#include "DataModel/Design/Design.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/LibPort.h"
#include "DataModel/Liberty/MemoryMgr.h"

#include <gtest/gtest.h>

#include <vector>

namespace {

class LibertyPortsTest : public ::testing::Test {
protected:
    void TearDown() override {
        if (inst_ != nullptr) {
            for (db::ObjectId pin_id : inst_->GetPinsArray()) {
                db::DesignMemoryMgr::DestroyPin(pin_id);
            }
            db::DesignMemoryMgr::DestroyInst(inst_->GetObjectId());
            inst_ = nullptr;
        }
        if (top_ != nullptr) {
            db::DesignMemoryMgr::DestroyModule(top_->GetObjectId());
            top_ = nullptr;
        }
        if (cell_ != nullptr) {
            db::LibertyMemoryMgr::DestroyLibCell(cell_->GetObjectId());
            cell_ = nullptr;
        }
        if (p1_ != nullptr) {
            db::LibertyMemoryMgr::DestroyLibPort(p1_->GetObjectId());
            p1_ = nullptr;
        }
        if (p2_ != nullptr) {
            db::LibertyMemoryMgr::DestroyLibPort(p2_->GetObjectId());
            p2_ = nullptr;
        }

        db::LibertyMemoryMgr liberty_mgr;
        liberty_mgr.Clear();

        db::Design::SetCurrentDesign(db::kInvalidId);
        if (design_ != nullptr) {
            db::DesignMemoryMgr::DestroyDesign(design_->GetObjectId());
            design_ = nullptr;
        }
    }

    db::Design*  design_ = nullptr;
    db::Module*  top_    = nullptr;
    db::Inst*    inst_   = nullptr;
    db::LibCell* cell_   = nullptr;
    db::LibPort* p1_     = nullptr;
    db::LibPort* p2_     = nullptr;
};

TEST_F(LibertyPortsTest, TwoPortLibCellPinArray) {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::Inst;
    using db::LibCell;
    using db::LibCellPortIterator;
    using db::LibPort;
    using db::LibertyMemoryMgr;
    using db::Module;

    LibertyMemoryMgr::EnsureManagers();
    ASSERT_NE(LibertyMemoryMgr::GetStringMgr(), nullptr);
    ASSERT_NE(LibertyMemoryMgr::GetIdArrayMgr(), nullptr);

    design_ = DesignMemoryMgr::CreateDesign();
    ASSERT_NE(design_, nullptr);
    Design::SetCurrentDesign(design_->GetObjectId());

    top_ = design_->CreateModule("top");
    ASSERT_NE(top_, nullptr);

    cell_ = LibertyMemoryMgr::CreateLibCell();
    p1_   = LibertyMemoryMgr::CreateLibPort();
    p2_   = LibertyMemoryMgr::CreateLibPort();
    ASSERT_NE(cell_, nullptr);
    ASSERT_NE(p1_, nullptr);
    ASSERT_NE(p2_, nullptr);

    auto ports = LibertyMemoryMgr::GetIdArrayMgr()->Allocate(2);
    ASSERT_EQ(ports.Capacity(), 2u);
    ports.PushBack(p1_->GetObjectId());
    ports.PushBack(p2_->GetObjectId());
    cell_->SetPorts(ports.Id(), 2);

    EXPECT_EQ(cell_->PortCount(), 2u);
    EXPECT_EQ(cell_->GetPortArray().Size(), 2u);

    {
        LibCellPortIterator       iter(cell_);
        std::vector<db::ObjectId> seen;
        while (iter.HasNext()) {
            LibPort* port = iter.Next();
            ASSERT_NE(port, nullptr);
            seen.push_back(port->GetObjectId());
        }
        ASSERT_EQ(seen.size(), 2u);
        EXPECT_EQ(seen[0], p1_->GetObjectId());
        EXPECT_EQ(seen[1], p2_->GetObjectId());
    }

    inst_ = top_->CreateInst(cell_, "u0");
    ASSERT_NE(inst_, nullptr);
    EXPECT_EQ(inst_->PinCount(), 2u);

    auto pins = inst_->GetPinsArray();
    ASSERT_EQ(pins.Size(), 2u);
    std::vector<db::ObjectId> pin_ids;
    for (db::ObjectId pin_id : pins) {
        EXPECT_NE(DesignMemoryMgr::GetPin(pin_id), nullptr);
        pin_ids.push_back(pin_id);
    }
    ASSERT_EQ(pin_ids.size(), 2u);
}

}  // namespace
