#ifndef DATAMODEL_DESIGN_PIN_H_
#define DATAMODEL_DESIGN_PIN_H_
#include "DataModel/Design/MemeryMgr.h"
#include "DataModel/Design/PinBase.h"
namespace db {
class Pin : public PinBase {
    friend class DesignMemoryMgr;

public:
    Pin();
    size_t GetSize() const override;

private:
};

}  // namespace db

#endif