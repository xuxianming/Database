#ifndef DATAMODEL_DESIGN_DESIGN_H_
#define DATAMODEL_DESIGN_DESIGN_H_

#include <string_view>
#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {
class Module;
class HInst;
class DFGGraph;

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
    // into the topmodule's top-level HInst list; otherwise it becomes a child.
    HInst* CreateHInst(HInst* parent, Module* template_module, std::string_view name);

    // Attaches the design's root DFG. Only a DFGGraph with type Module can be
    // attached. The graph is owned by the DesignMemoryMgr object graph.
    bool AttachDFG(DFGGraph* graph);
    DFGGraph* GetDFG() const;

    DesignStringMgr*       GetStringMgr() { return m_string_mgr; }
    const DesignStringMgr* GetStringMgr() const { return m_string_mgr; }
    DesignIdArrayMgr*      GetIdArrayMgr() { return m_idarray_mgr; }


private:
    inline void SetStringMgr(DesignStringMgr* mgr) { m_string_mgr = mgr; }
    inline void SetIdArrayMgr(DesignIdArrayMgr* mgr) { m_idarray_mgr = mgr; }

private:
    ObjectId m_top_module_id{kInvalidId};
    ObjectId m_dfg_graph_id{kInvalidId};

    DesignStringMgr*  m_string_mgr{nullptr};
    DesignIdArrayMgr* m_idarray_mgr{nullptr};
    static ObjectId   m_current_design_id;
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_DESIGN_H_
