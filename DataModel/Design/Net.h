#ifndef DATAMODEL_DESIGN_NET_H_
#define DATAMODEL_DESIGN_NET_H_

#include <cstdint>
#include "DataModel/Design/MemeryMgr.h"
#include "DataModel/Design/NetBase.h"
namespace db {
class Net : public NetBase {
    friend class DesignMemoryMgr;

public:
    Net();
    size_t GetSize() const override;

private:
    uint32_t m_bit_id{};
};

}  // namespace db

#endif