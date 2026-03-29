#ifndef DATAMODEL_DESIGN_INST_H_
#define DATAMODEL_DESIGN_INST_H_

#include "DataModel/Design/InstBase.h"
#include "DataModel/Design/MemeryMgr.h"
namespace db {
class Inst : public InstBase {
    friend class DesignMemoryMgr;

public:
    Inst();
    size_t GetSize() const override;

private:
};

}  // namespace db

#endif