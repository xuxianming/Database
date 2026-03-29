#ifndef DATAMODEL_DESIGN_HINST_H_
#define DATAMODEL_DESIGN_HINST_H_

#include "DataModel/Design/InstBase.h"
#include "DataModel/Design/MemeryMgr.h"
namespace db {
class HInst : public InstBase {
    friend class DesignMemoryMgr;

public:
    HInst();
    size_t GetSize() const override;

private:

    uint32_t m_insts_id;
    uint32_t m_hinsts_id;
};

}  // namespace db

#endif