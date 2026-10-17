#ifndef DATAMODEL_DESIGN_MODULE_H_
#define DATAMODEL_DESIGN_MODULE_H_
#include <string_view>
#include <unordered_set>
#include "DataModel/Design/Type.h"
#include "DataModel/Enums.h"
#include "DataModel/Object.h"

namespace db {
class Design;
class Inst;
class HInst;
class Net;
class Pin;
class Port;
class BusPort;
class BusNet;

class Module : public Object {
    friend class HInstHInstIterator;
    friend class HInstInstIterator;
    friend class HInstNetIterator;
    friend class HInstHNetIterator;
    friend class ModuleInstIterator;
    friend class ModuleHInstIterator;
    friend class ModuleNetIterator;
    friend class ModuleHNetIterator;
    friend class ModulePortIterator;
    friend class Design;
    friend class HInst;

private:
public:
    Module();
    ~Module();
    std::string_view GetName() const override;
    std::string      PathName() override;

    Design*  GetMasterDesign() const;
    Module*  GetTopModule() const;
    HInst*   GetViewHInst() const;
    uint32_t GetViewHInstCount() const;
    void     Unfold(bool copy_module = true);
    Inst*    CreateInst(LibCell* lib_cell, const std::string& name = "");
    Inst*    CreateInst(ObjectId lib_cell_id, const std::string& name = "");
    Inst*    CreateInst(LogicType type, const std::string& name = "");
    HInst*   CreateHInst(Module* template_module, const std::string& name = "");
    HInst*   CreateHInst(ObjectId template_module_id, const std::string& name = "");
    HInst*   CreateHInst(OperatorType type, const std::string& name = "");
    Module*  Clone(const std::string& name = "") const;
    bool     Rename(std::string_view name);

    // Name -> object lookup within this module. The index is built lazily from
    // the module's Inst/HInst lists and invalidated on mutation. Duplicate
    // names resolve to the object most recently linked (list head).
    Inst*  FindInst(std::string_view name) const;
    HInst* FindHInst(std::string_view name) const;

private:
    friend class DesignMemoryMgr;

    static size_t ComputeHash(const std::string& name) { return std::hash<std::string>{}(name); }

    void LinkInst(Inst* inst);
    void LinkHInst(HInst* hinst);
    void UnlinkInst(Inst* inst);
    void UnlinkHInst(HInst* hinst);

private:
    ObjectId     m_design_id{kInvalidId};
    ObjectId     m_template_id{kInvalidId};
    ObjectId     m_design_next_module_id{kInvalidId};
    ObjectId     m_netbus_head_id{kInvalidId};
    ObjectId     m_portbus_head_id{kInvalidId};
    ObjectId     m_view_hinst_id{kInvalidId};
    ObjectId     m_port_head_id{kInvalidId};
    ObjectId     m_net_head_id{kInvalidId};
    ObjectId     m_hnet_head_id{kInvalidId};
    ObjectId     m_inst_head_id{kInvalidId};
    ObjectId     m_hinst_head_id{kInvalidId};
    ObjectId     m_next_module_id{kInvalidId};
    uint32_t     m_is_top : 1;
    uint32_t     m_clone_count : 15;
    OperatorType m_operator_type{OperatorType::COUNT};
};

// Side index helpers (defined in Module.cpp). The index is a separate static
// map so the raw ObjectTable layout of Module stays unchanged.
void InvalidateModuleNameIndex(ObjectId module_id);
void ClearModuleNameIndexes();

class ModuleInstIterator : public Iterator<Inst> {
public:
    ModuleInstIterator(Module* module);
    ~ModuleInstIterator() override = default;
    Inst* Next() override;
    bool  HasNext() const override { return m_next_inst_id != kInvalidId; }

private:
    ObjectId m_next_inst_id{kInvalidId};
};

class ModuleHInstIterator : public Iterator<HInst> {
public:
    ModuleHInstIterator(Module* module);
    ~ModuleHInstIterator() override = default;
    HInst* Next() override;
    bool   HasNext() const override { return m_next_hinst_id != kInvalidId; }

private:
    ObjectId m_next_hinst_id{kInvalidId};
};

class ModuleNetIterator : public Iterator<Net> {
public:
    ModuleNetIterator(Module* module);
    ~ModuleNetIterator() override = default;
    Net* Next() override;
    bool HasNext() const override { return m_next_net_id != kInvalidId; }

private:
    ObjectId m_next_net_id{kInvalidId};
};

class ModuleHNetIterator : public Iterator<HNet> {
public:
    ModuleHNetIterator(Module* module);
    ~ModuleHNetIterator() override = default;
    HNet* Next() override;
    bool  HasNext() const override { return m_next_hnet_id != kInvalidId; }

private:
    ObjectId m_next_hnet_id{kInvalidId};
};

class ModulePortIterator : public Iterator<ModulePort> {
public:
    ModulePortIterator(Module* module);
    ~ModulePortIterator() override = default;
    ModulePort* Next() override;
    bool        HasNext() const override { return m_next_port_id != kInvalidId; }

private:
    ObjectId m_next_port_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_MODULE_H_