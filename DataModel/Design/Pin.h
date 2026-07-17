#ifndef DATAMODEL_DESIGN_PIN_H_
#define DATAMODEL_DESIGN_PIN_H_
#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"

namespace db {
class Pin : public Object {
    friend class DesignMemoryMgr;
    friend class Net;
    friend class Inst;
    friend class HInst;
    friend class HInstPinIterator;

public:
    Pin()           = default;
    ~Pin() override = default;
    size_t      GetSize() const override;
    ModulePort* GetModulePort() const;
    LibPort*    GetLibPort() const;
    Net*        GetNet() const;
    HNet*       GetHNet() const;
    Inst*       GetInst() const;
    bool        IsIoPin() const { return m_is_iopin; }

    // Connect api
    void ConnectNet(Net* net);
    void DisConnectNet();
    void ConnectHNet(HNet* hnet);
    void DisConnectHNet();

private:
    static Pin* Create(Inst* inst, ObjectId port_id);
    static Pin* Create(Inst* inst, LibPort* port);
    static Pin* Create(ObjectId inst_id, ObjectId port_id);
    static Pin* Create(ObjectId inst_id, LibPort* port);

private:
    ObjectId m_hnet_id{kInvalidId};
    ObjectId m_net_id{kInvalidId};
    ObjectId m_port_id{kInvalidId};
    ObjectId m_inst_id{kInvalidId};
    ObjectId m_next_pin_id{kInvalidId};
    ObjectId m_prev_pin_id{kInvalidId};
    ObjectId m_hnet_next_pin_id{kInvalidId};
    ObjectId m_hnet_prev_pin_id{kInvalidId};
    uint32_t m_is_iopin : 1;
};

}  // namespace db

#endif