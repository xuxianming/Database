// Standalone integration test: Liberty managers + a 2-port LibCell/Inst pin array.
//
// Build & run:
//   cmake --build <build> --target liberty_ports_test && ./bin/liberty_ports_test

#include "DataModel/Design/Design.h"
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Liberty/LibCell.h"
#include "DataModel/Liberty/LibPort.h"
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
    using db::Inst;
    using db::LibCell;
    using db::LibCellPortIterator;
    using db::LibPort;
    using db::LibertyMemoryMgr;
    using db::Module;

    LibertyMemoryMgr::EnsureManagers();
    CHECK(LibertyMemoryMgr::GetStringMgr() != nullptr);
    CHECK(LibertyMemoryMgr::GetIdArrayMgr() != nullptr);

    Design* design = DesignMemoryMgr::CreateDesign();
    CHECK(design != nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    Module* top = design->CreateModule("top");
    CHECK(top != nullptr);

    LibCell* cell = LibertyMemoryMgr::CreateLibCell();
    LibPort* p1   = LibertyMemoryMgr::CreateLibPort();
    LibPort* p2   = LibertyMemoryMgr::CreateLibPort();
    CHECK(cell != nullptr);
    CHECK(p1 != nullptr);
    CHECK(p2 != nullptr);

    auto ports = LibertyMemoryMgr::GetIdArrayMgr()->Allocate(2);
    CHECK(ports.Capacity() == 2);
    ports.PushBack(p1->GetObjectId());
    ports.PushBack(p2->GetObjectId());
    cell->SetPorts(ports.Id(), 2);

    CHECK(cell->PortCount() == 2);
    CHECK(cell->GetPortArray().Size() == 2);

    {
        LibCellPortIterator           iter(cell);
        std::vector<db::ObjectId>     seen;
        while (iter.HasNext()) {
            LibPort* port = iter.Next();
            CHECK(port != nullptr);
            seen.push_back(port->GetObjectId());
        }
        CHECK(seen.size() == 2);
        CHECK(seen[0] == p1->GetObjectId());
        CHECK(seen[1] == p2->GetObjectId());
    }

    Inst* inst = top->CreateInst(cell, "u0");
    CHECK(inst != nullptr);
    CHECK(inst->PinCount() == 2);

    auto pins = inst->GetPinsArray();
    CHECK(pins.Size() == 2);
    std::vector<db::ObjectId> pin_ids;
    for (db::ObjectId pin_id : pins) {
        CHECK(DesignMemoryMgr::GetPin(pin_id) != nullptr);
        pin_ids.push_back(pin_id);
    }
    CHECK(pin_ids.size() == 2);

    for (db::ObjectId pin_id : pin_ids) {
        DesignMemoryMgr::DestroyPin(pin_id);
    }
    DesignMemoryMgr::DestroyInst(inst->GetObjectId());
    DesignMemoryMgr::DestroyModule(top->GetObjectId());
    LibertyMemoryMgr::DestroyLibCell(cell->GetObjectId());
    LibertyMemoryMgr::DestroyLibPort(p1->GetObjectId());
    LibertyMemoryMgr::DestroyLibPort(p2->GetObjectId());

    LibertyMemoryMgr liberty_mgr;
    liberty_mgr.Clear();

    Design::SetCurrentDesign(db::kInvalidId);
    DesignMemoryMgr::DestroyDesign(design->GetObjectId());

    std::printf("liberty_ports_test: all checks passed\n");
    return 0;
}
