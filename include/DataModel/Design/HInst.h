#ifndef DATAMODEL_DESIGN_HINST_H_
#define DATAMODEL_DESIGN_HINST_H_

#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"

namespace db {

/**
 *  HInst is a view of module, it is used to represent the hierarchical structure of design,
 */
class HInst : public Object {
    friend class Module;
    friend class Design;
    // iterators
    friend class HInstHInstIterator;
    friend class HInstInstIterator;
    friend class HInstHPinIterator;
    friend class HInstNetIterator;
    friend class HInstHNetIterator;
    friend class ModuleHInstIterator;

public:
    HInst()           = default;
    ~HInst() override = default;
    size_t  GetSize() const override;
    std::string_view GetName() const override;
    std::string PathName() override;
    bool        Rename(std::string_view name);
    Design* GetMasterDesign() const;
    Module* GetModule() const;
    HInst*  GetUpperHInst() const;
    Module* GetTemplateModule() const;
    void    Unfold();
    bool    IsUnfolded() const { return m_unfold; }

private:
    void HierarchyExpansion(Module* template_module);

    static HInst* Create(Module* module, Module* template_module);
    static HInst* Create(ObjectId module_id, ObjectId template_module_id);
    static HInst* Create(Module* module, OperatorType type);

private:
    ObjectId     m_module_id{kInvalidId};
    ObjectId     m_template_id{kInvalidId};
    ObjectId     m_hpin_head_id{kInvalidId};
    ObjectId     m_next_hinst_id{kInvalidId};
    ObjectNameId m_path_name_id{kInvalidId};
    uint32_t     m_unfold : 1 {0};
};

class HInstHInstIterator : public Iterator<HInst> {
public:
    HInstHInstIterator(HInst* hinst);
    ~HInstHInstIterator() override = default;
    HInst* Next() override;

private:
    Module*  m_template_module;
    ObjectId m_next_hinst_id{kInvalidId};
};

class HInstInstIterator : public Iterator<Inst> {
public:
    HInstInstIterator(HInst* hinst);
    ~HInstInstIterator() override = default;
    Inst* Next() override;

private:
    Module*  m_template_module;
    ObjectId m_next_inst_id{kInvalidId};
};

class HInstNetIterator : public Iterator<Net> {
public:
    HInstNetIterator(HInst* hinst);
    ~HInstNetIterator() override = default;
    Net* Next() override;

private:
    Module*  m_template_module;
    ObjectId m_next_net_id{kInvalidId};
};

class HInstHNetIterator : public Iterator<HNet> {
public:
    HInstHNetIterator(HInst* hinst);
    ~HInstHNetIterator() override = default;
    HNet* Next() override;

private:
    Module*  m_template_module;
    ObjectId m_next_hnet_id{kInvalidId};
};

class HInstHPinIterator : public Iterator<HPin> {
public:
    HInstHPinIterator(HInst* hinst);
    ~HInstHPinIterator() override = default;
    HPin* Next() override;

private:
    HInst*   m_hinst;
    ObjectId m_next_hpin_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_HINST_H_