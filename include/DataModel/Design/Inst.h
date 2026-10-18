#ifndef DATAMODEL_DESIGN_INST_H_
#define DATAMODEL_DESIGN_INST_H_

#include "DataModel/Design/Enums.h"
#include "DataModel/Design/Type.h"
#include "DataModel/Enums.h"
#include "DataModel/Object.h"

namespace db {
class Port;
class LibPort;
class Module;
class HInst;
class LibCell;
class Pin;
class ModulePort;
using InstPinsArray = IdArray<ObjectId, InstPinsArrayId>;
class Inst : public Object {
    friend class HInst;
    friend class Module;
    friend class HInstInstIterator;
    friend class InstPinIterator;
    friend class ModuleInstIterator;

public:
    Inst()           = default;
    ~Inst() override = default;
    size_t               GetSize() const override;
    std::string_view     GetName() const override;
    std::string          PathName() override;
    bool                 Rename(std::string_view name);
    Module*              GetModule() const;
    HInst*               GetHInst() const;
    LibCell*             GetLibCell() const;
    Design*              GetMasterDesign() const;
    void                 Connect(LogicPort port, Net* net);
    uint32_t             PinCount() const;
    InstPinsArray GetPinsArray() const;

private:
    static Inst* Create(Module* module, Inst* template_inst);
    static Inst* Create(Module* module, ObjectId libcell_id);
    static Inst* Create(Module* module, LibCell* libcell);
    static Inst* Create(Module* module, LogicType type);
    void         InitPins();

private:
    ObjectId        m_module_id{kInvalidId};
    ObjectId        m_hinst_id{kInvalidId};
    ObjectId        m_libcell_id{kInvalidId};
    ObjectId        m_macro_id{kInvalidId};
    ObjectId        m_module_inst_id{kInvalidId};
    ObjectId        m_next_inst_id{kInvalidId};
    ObjectId        m_prev_inst_id{kInvalidId};
    InstPinsArrayId m_pins_array_id{kInvalidId};
    InstType        m_inst_type{InstType::kTotalCount};
    LogicType       m_function_type{LogicType::COUNT};
};

class InstPinIterator : public Iterator<Pin> {
public:
    InstPinIterator(Inst* inst);
    ~InstPinIterator() override = default;
    Pin* Next() override;
    bool HasNext() const override { return m_index < m_pins_array.Size(); }

private:
    IdArray<ObjectId, InstPinsArrayId> m_pins_array;
    size_t                             m_index{0};
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_INST_H_