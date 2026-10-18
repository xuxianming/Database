#ifndef DATAMODEL_DESIGN_HINST_H_
#define DATAMODEL_DESIGN_HINST_H_

#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"

namespace db {

/**
 *  HInst is a hierarchy occurrence of a module; it owns the occurrence-level
 *  child HInst/Inst/HNet/HPin containers.
 */
class HInst : public Object {
    friend class Design;
    friend class DesignMemoryMgr;
    // iterators
    friend class DesignTopHInstIterator;
    friend class HInstHInstIterator;
    friend class HInstInstIterator;
    friend class HInstHPinIterator;
    friend class HInstHNetIterator;

public:
    HInst()           = default;
    ~HInst() override = default;
    size_t  GetSize() const override;
    std::string_view GetName() const override;
    std::string PathName() override;
    bool        Rename(std::string_view name);
    Design* GetDesign() const;
    Module* GetModule() const;
    // The containing/upper HInst; null for a top-level occurrence.
    HInst*  GetHInst() const;
    Module* GetTemplateModule() const;

private:
    // Hierarchy tree occurrence. m_module_id always points at the design's top
    // module; m_template_id is the module definition this occurrence instantiates.
    ObjectId     m_design_id{kInvalidId};
    ObjectId     m_module_id{kInvalidId};
    ObjectId     m_template_id{kInvalidId};
    ObjectId     m_upper_hinst_id{kInvalidId};
    ObjectId     m_child_hinst_head_id{kInvalidId};
    ObjectId     m_next_hinst_id{kInvalidId};
    ObjectId     m_hpin_head_id{kInvalidId};
    // HInst-owned occurrence lists. They stay empty until recursive hierarchy
    // expansion clones the template module's internal objects per occurrence.
    ObjectId     m_inst_head_id{kInvalidId};
    ObjectId     m_hnet_head_id{kInvalidId};
};

class HInstHInstIterator : public Iterator<HInst> {
public:
    HInstHInstIterator(HInst* hinst);
    ~HInstHInstIterator() override = default;
    HInst* Next() override;
    bool   HasNext() const override { return m_next_hinst_id != kInvalidId; }

private:
    ObjectId m_next_hinst_id{kInvalidId};
};

class HInstInstIterator : public Iterator<Inst> {
public:
    HInstInstIterator(HInst* hinst);
    ~HInstInstIterator() override = default;
    Inst* Next() override;
    bool  HasNext() const override { return m_next_inst_id != kInvalidId; }

private:
    ObjectId m_next_inst_id{kInvalidId};
};

class HInstHNetIterator : public Iterator<HNet> {
public:
    HInstHNetIterator(HInst* hinst);
    ~HInstHNetIterator() override = default;
    HNet* Next() override;
    bool  HasNext() const override { return m_next_hnet_id != kInvalidId; }

private:
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
