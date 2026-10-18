#ifndef DATAMODEL_DESIGN_DESIGN_H_
#define DATAMODEL_DESIGN_DESIGN_H_

#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {
class Module;

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
    void           Unfold(bool copy_module = false);
    Module*        CreateModule(const std::string& name = "");
    Module*        CreateModule(OperatorType type, const std::string& name = "");

    DesignStringMgr*       GetStringMgr() { return m_string_mgr; }
    const DesignStringMgr* GetStringMgr() const { return m_string_mgr; }
    DesignIdArrayMgr*      GetIdArrayMgr() { return m_idarray_mgr; }


private:
    inline void SetStringMgr(DesignStringMgr* mgr) { m_string_mgr = mgr; }
    inline void SetIdArrayMgr(DesignIdArrayMgr* mgr) { m_idarray_mgr = mgr; }

private:
    ObjectId m_top_module_id{kInvalidId};
    // list of modules in design, for fast traversal of all modules in design
    ObjectId m_module_head_id{kInvalidId};

    DesignStringMgr*  m_string_mgr{nullptr};
    DesignIdArrayMgr* m_idarray_mgr{nullptr};
    static ObjectId   m_current_design_id;
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_DESIGN_H_