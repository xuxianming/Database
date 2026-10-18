#ifndef DATAMODEL_DESIGN_MODULEPORT_H_
#define DATAMODEL_DESIGN_MODULEPORT_H_
#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
#include "DataModel/Port.h"
namespace db {
class ModulePort : public Port {
    friend class Module;
    friend class ModulePortIterator;

public:
    ModulePort()           = default;
    ~ModulePort() override = default;
    size_t  GetSize() const override;
    Module* GetModule() const;
    Design* GetMasterDesign() const;
    Pin*    GetIoPin() const;
    Net*    GetNet() const;
    HNet*   GetHNet() const;

private:
    ObjectId m_module_id{kInvalidId};
    ObjectId m_upper_hpin_id{kInvalidId};
    ObjectId m_io_pin_id{kInvalidId};
    ObjectId m_next_port_id{kInvalidId};
};

}  // namespace db

#endif  // DATAMODEL_DESIGN_MODULEPORT_H_
