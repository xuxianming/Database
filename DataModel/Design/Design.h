#ifndef DATAMODEL_DESIGN_DESIGN_H_
#define DATAMODEL_DESIGN_DESIGN_H_
#include "DataModel/Design/DesignObject.h"
namespace db {
class StringMgrMaintainer;
class StringMgr;
class Design : public DesignObject {
    friend class StringMgrMaintainer;

public:
    Design()           = default;
    ~Design() override = default;
    size_t GetSize() const override;

private:
    inline void SetStringMgr(StringMgr* mgr) { m_string_mgr = mgr; }

private:
    NetArrayId     m_nets_id{};
    HNetArrayId    m_hnets_id{};
    PortArrayId    m_ports_id{};
    InstArrayId    m_insts_id{};
    HInstArrayId   m_hinsts_id{};
    BusNetArrayId  m_bus_nets_id{};
    BusPortArrayId m_bus_ports_id{};
    StringMgr*     m_string_mgr{nullptr};
};

}  // namespace db

#endif