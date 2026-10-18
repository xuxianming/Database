
#include "DataModel/Design/Inst.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/HInst.h"
#include "DataModel/Design/Module.h"
#include "DataModel/Design/Pin.h"
#include "DataModel/Liberty/LibPort.h"

namespace db {
size_t Inst::GetSize() const { return sizeof(Inst); }

std::string_view Inst::GetName() const {
    Module* module = m_module_id == kInvalidId ? nullptr : DesignMemoryMgr::GetModule(m_module_id);
    if (module == nullptr) {
        return {};
    }
    Design* design = module->GetDesign();
    if (design == nullptr || design->GetStringMgr() == nullptr) {
        return {};
    }
    return design->GetStringMgr()->GetString(m_name_id);
}

std::string Inst::PathName() {
    const std::string name(GetName());
    std::string       base;
    if (HInst* hinst = GetHInst(); hinst != nullptr) {
        base = hinst->PathName();
    } else if (Module* module =
                   m_module_id == kInvalidId ? nullptr : DesignMemoryMgr::GetModule(m_module_id);
               module != nullptr) {
        base = std::string(module->GetName());
    }
    if (base.empty()) {
        return name;
    }
    if (name.empty()) {
        return base;
    }
    return base + "/" + name;
}

bool Inst::Rename(std::string_view name) {
    Module* module = m_module_id == kInvalidId ? nullptr : DesignMemoryMgr::GetModule(m_module_id);
    Design* design = module == nullptr ? nullptr : module->GetDesign();
    if (design == nullptr || design->GetStringMgr() == nullptr) {
        return false;
    }
    DesignStringMgr*   mgr    = design->GetStringMgr();
    const ObjectNameId old_id = m_name_id;
    const ObjectNameId new_id = mgr->AddString(name);
    m_name_id                 = new_id;
    if (old_id != 0) {
        mgr->ReleaseString(old_id);
    }
    InvalidateModuleNameIndex(module->GetObjectId());
    return true;
}

Module* Inst::GetModule() const {
    if (m_top_module_id != kInvalidId) {
        if (Module* top_module = DesignMemoryMgr::GetModule(m_top_module_id); top_module != nullptr) {
            return top_module;
        }
    }
    if (Design* design = GetDesign(); design != nullptr) {
        return design->GetTopModule();
    }
    return nullptr;
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

Design* Inst::GetDesign() const {
    if (m_design_id != kInvalidId) {
        if (Design* design = DesignMemoryMgr::GetDesign(m_design_id); design != nullptr) {
            return design;
        }
    }
    if (m_top_module_id != kInvalidId) {
        if (Module* top_module = DesignMemoryMgr::GetModule(m_top_module_id);
            top_module != nullptr) {
            return top_module->GetDesign();
        }
    }
    if (Module* owner =
            m_module_id == kInvalidId ? nullptr : DesignMemoryMgr::GetModule(m_module_id);
        owner != nullptr) {
        return owner->GetDesign();
    }
    return nullptr;
}

void Inst::Connect(LogicPort port, Net* net) {}

uint32_t Inst::PinCount() const {
    if (m_pins_array_id == kInvalidId) {
        return 0;
    }
    auto* array_mgr = GetDesign()->GetIdArrayMgr();
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
    auto* array_mgr = GetDesign()->GetIdArrayMgr();
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
    Design* design = module->GetDesign();
    if (design == nullptr || design->GetIdArrayMgr() == nullptr) {
        DesignMemoryMgr::DestroyInst(inst->GetObjectId());
        return nullptr;
    }
    inst->m_module_id  = module->GetObjectId();
    inst->m_design_id  = design->GetObjectId();
    inst->m_hinst_id   = kInvalidId;
    if (Module* top_module = design->GetTopModule(); top_module != nullptr) {
        inst->m_top_module_id = top_module->GetObjectId();
    }
    inst->m_libcell_id            = libcell->GetObjectId();
    LibCellPortsArray ports_array = libcell->GetPortArray();
    InstPinsArray array(design->GetIdArrayMgr()->Allocate(ports_array.Size()));
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
    auto* array_mgr = inst->GetDesign()->GetIdArrayMgr();
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