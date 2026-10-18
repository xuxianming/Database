// Tests for the Pin/Net/HNet/HPin connectivity split:
// - Net connects Pin only.
// - Pin can connect both Net and HNet.
// - HNet can connect both Pin and HPin.

#include "DataModel/Design/Design.h"
#include "DataModel/Design/HNet.h"
#include "DataModel/Design/HPin.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Design/Net.h"
#include "DataModel/Design/Pin.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/LibPort.h"
#include "DataModel/Liberty/MemoryMgr.h"

#include <gtest/gtest.h>

TEST(ConnectivityTest, PinConnectsToNetAndHNet) {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::HNet;
    using db::HPin;
    using db::Inst;
    using db::LibCell;
    using db::LibPort;
    using db::LibertyMemoryMgr;
    using db::Module;
    using db::Net;
    using db::Pin;

    LibertyMemoryMgr::EnsureManagers();
    Design* design = DesignMemoryMgr::CreateDesign();
    ASSERT_NE(design, nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    Module* top = design->CreateModule("top");
    ASSERT_NE(top, nullptr);

    LibCell* cell = LibertyMemoryMgr::CreateLibCell();
    LibPort* port = LibertyMemoryMgr::CreateLibPort();
    ASSERT_NE(cell, nullptr);
    ASSERT_NE(port, nullptr);
    auto ports = LibertyMemoryMgr::GetIdArrayMgr()->Allocate(1);
    ports.PushBack(port->GetObjectId());
    cell->SetPorts(ports.Id(), 1);

    Inst* inst = top->CreateInst(cell, "u0");
    ASSERT_NE(inst, nullptr);
    ASSERT_EQ(inst->PinCount(), 1u);
    Pin* pin = DesignMemoryMgr::GetPin(inst->GetPinsArray()[0]);
    ASSERT_NE(pin, nullptr);

    Net*  net  = DesignMemoryMgr::CreateNet();
    HNet* hnet = DesignMemoryMgr::CreateHNet();
    HPin* hpin = DesignMemoryMgr::CreateHPin();
    ASSERT_NE(net, nullptr);
    ASSERT_NE(hnet, nullptr);
    ASSERT_NE(hpin, nullptr);

    pin->ConnectNet(net);
    pin->ConnectHNet(hnet);
    EXPECT_EQ(pin->GetNet(), net);
    EXPECT_EQ(pin->GetHNet(), hnet);

    hpin->ConnectUpperHNet(hnet);
    EXPECT_EQ(hpin->GetUpperHNet(), hnet);

    const db::ObjectId pin_id  = pin->GetObjectId();
    const db::ObjectId hpin_id = hpin->GetObjectId();
    const db::ObjectId hnet_id = hnet->GetObjectId();
    const db::ObjectId net_id  = net->GetObjectId();

    pin->DisConnectHNet();
    pin->DisConnectNet();
    hpin->DisConnectUpperHNet();

    DesignMemoryMgr::DestroyHPin(hpin_id);
    DesignMemoryMgr::DestroyHNet(hnet_id);
    DesignMemoryMgr::DestroyNet(net_id);
    DesignMemoryMgr::DestroyPin(pin_id);
    DesignMemoryMgr::DestroyInst(inst->GetObjectId());
    DesignMemoryMgr::DestroyModule(top->GetObjectId());
    LibertyMemoryMgr::DestroyLibCell(cell->GetObjectId());
    LibertyMemoryMgr::DestroyLibPort(port->GetObjectId());

    LibertyMemoryMgr liberty_mgr;
    liberty_mgr.Clear();
    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(design->GetObjectId());
}
