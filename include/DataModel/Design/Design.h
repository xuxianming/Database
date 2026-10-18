#ifndef DATAMODEL_DESIGN_DESIGN_H_
#define DATAMODEL_DESIGN_DESIGN_H_

#include <string_view>
#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {
class Module;
class HInst;

class Design : public Object {
    friend class DesignMemoryMgr;

public:
    Design()           = default;
    ~Design() override = default;
    size_t         GetSize() const override;
    uint32_t       GetMaxFanout() const;
    Module*        GetTopModule() const;
    void           SetTopModule(ObjectId top_module_id);
    static void    SetCurrentDesign(ObjectId designId);
    static Design* GetCurrentDesign();
    Module*        CreateModule(const std::string& name = "");
    Module*        CreateModule(OperatorType type, const std::string& name = "");

    // Creates a hierarchy occurrence of template_module. A null parent links it
    // into the design's top list; otherwise it becomes a child of parent.
    HInst* CreateHInst(HInst* parent, Module* template_module, std::string_view name);

    ObjectId GetTopHInstHead() const { return m_top_hinst_head_id; }

    DesignStringMgr*       GetStringMgr() { return m_string_mgr; }
    const DesignStringMgr* GetStringMgr() const { return m_string_mgr; }
    DesignIdArrayMgr*      GetIdArrayMgr() { return m_idarray_mgr; }


private:
    inline void SetStringMgr(DesignStringMgr* mgr) { m_string_mgr = mgr; }
    inline void SetIdArrayMgr(DesignIdArrayMgr* mgr) { m_idarray_mgr = mgr; }

private:
    ObjectId m_top_module_id{kInvalidId};
    // top-level hierarchy occurrences
    ObjectId m_top_hinst_head_id{kInvalidId};

    DesignStringMgr*  m_string_mgr{nullptr};
    DesignIdArrayMgr* m_idarray_mgr{nullptr};
    static ObjectId   m_current_design_id;
};

class DesignTopHInstIterator : public Iterator<HInst> {
public:
    DesignTopHInstIterator(Design* design);
    ~DesignTopHInstIterator() override = default;
    HInst* Next() override;
    bool   HasNext() const override { return m_next_hinst_id != kInvalidId; }

private:
    ObjectId m_next_hinst_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_DESIGN_H_
