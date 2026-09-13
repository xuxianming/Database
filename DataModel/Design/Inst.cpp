
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Design/Pin.h"
#include "DataModel/Liberty/LibPort.h"
#include "Inst.h"

namespace db {
size_t Inst::GetSize() const { return sizeof(Inst); }

Module* Inst::GetModule() const {
    if (m_module_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetModule(m_module_id);
}

HInst* Inst::GetHInst() const {
    if (m_hinst_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetHInst(m_hinst_id);
}

LibCell* Inst::GetLibCell() const {
    if (m_libcell_id == kInvalidId) {
        return nullptr;
    }
    return LibertyMemoryMgr::GetLibCell(m_libcell_id);
}

Design* Inst::GetMasterDesign() const {
    Module* module = GetModule();
    if (module != nullptr) {
        return module->GetMasterDesign();
    }
    return nullptr;
}

void Inst::Connect(LogicPort port, Net* net) {}

uint32_t Inst::PinCount() const {
    if (m_pins_array_id == kInvalidId) {
        return 0;
    }
    auto* array_mgr = GetMasterDesign()->GetIdArrayMgr();
    if (array_mgr == nullptr) {
        return 0;
    }
    auto pins_array = array_mgr->Get(m_pins_array_id);
    return pins_array.Size();
}

InstPinsArray Inst::GetPinsArray() const {
    if (m_pins_array_id == kInvalidId) {
        static const InstPinsArray empty_array;
        return empty_array;
    }
    auto* array_mgr = GetMasterDesign()->GetIdArrayMgr();
    if (array_mgr == nullptr) {
        static const InstPinsArray empty_array;
        return empty_array;
    }
    return array_mgr->Get(m_pins_array_id);
}

Inst* Inst::Create(Module* module, Inst* template_inst) {
    if (module == nullptr || template_inst == nullptr) {
        return nullptr;
    }
    return Create(module, template_inst->m_libcell_id);
}

Inst* Inst::Create(Module* module, ObjectId libcell_id) {
    if (module == nullptr) {
        return nullptr;
    }
    LibCell* libcell = LibertyMemoryMgr::GetLibCell(libcell_id);
    if (libcell == nullptr) {
        return nullptr;
    }
    return Create(module, libcell);
}

Inst* Inst::Create(Module* module, LibCell* libcell) {
    if (module == nullptr || libcell == nullptr) {
        return nullptr;
    }
    Inst* inst = DesignMemoryMgr::CreateInst();
    if (inst == nullptr) {
        return nullptr;
    }
    inst->m_module_id             = module->GetObjectId();
    inst->m_libcell_id            = libcell->GetObjectId();
    LibCellPortsArray ports_array = libcell->GetPortArray();
    InstPinsArray array(module->GetMasterDesign()->GetIdArrayMgr()->Allocate(ports_array.Size()));
    for (ObjectId port_id : ports_array) {
        Pin* pin = Pin::Create(inst, port_id);
        if (pin == nullptr) {
            continue;
        }
        array.PushBack(pin->GetObjectId());
    }
    inst->m_pins_array_id = array.Id();
    return inst;
}

Inst* Inst::Create(Module* module, LogicType type) {
    if (module == nullptr || type == LogicType::COUNT) {
        return nullptr;
    }
    LibCell* libcell = LibertyMemoryMgr::GetLibCellByLogicType(type);
    if (libcell == nullptr) {
        return nullptr;
    }
    return Create(module, libcell);
}

void Inst::InitPins() {}

InstPinIterator::InstPinIterator(Inst* inst) {
    if (inst == nullptr) {
        return;
    }
    auto* array_mgr = inst->GetMasterDesign()->GetIdArrayMgr();
    if (array_mgr == nullptr) {
        return;
    }
    m_pins_array = array_mgr->Get(inst->m_pins_array_id);
}

Pin* InstPinIterator::Next() {
    if (m_index >= m_pins_array.Size()) {
        return nullptr;
    }
    ObjectId pin_id = m_pins_array[m_index++];
    if (pin_id == kInvalidId) {
        return nullptr;
    }
    return DesignMemoryMgr::GetPin(pin_id);
}

}  // namespace db