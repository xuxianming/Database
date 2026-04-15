#ifndef DATAMODEL_DESIGN_MODULE_H_
#define DATAMODEL_DESIGN_MODULE_H_
#include "DataModel/Design/Type.h"
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
private:
public:
    Module();
    ~Module();

private:
    ObjectId       m_design_id;
    NetArrayId     m_nets_id{};
    HNetArrayId    m_hnets_id{};
    PortArrayId    m_ports_id{};
    InstArrayId    m_insts_id{};
    HInstArrayId   m_hinsts_id{};
    BusNetArrayId  m_bus_nets_id{};
    BusPortArrayId m_bus_ports_id{};
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_MODULE_H_