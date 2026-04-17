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
    ObjectId       m_design_id{kInvalidId};
    NetArrayId     m_nets_id{kInvalidId};
    PortArrayId    m_ports_id{kInvalidId};
    InstArrayId    m_insts_id{kInvalidId};
    HInstArrayId   m_hinsts_id{kInvalidId};
    BusNetArrayId  m_bus_nets_id{kInvalidId};
    BusPortArrayId m_bus_ports_id{kInvalidId};
    ObjectId       m_template_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_MODULE_H_