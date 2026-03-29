#ifndef DATABASE_OBJECT_H_
#define DATABASE_OBJECT_H_
#include <cstdint>
#include <string>
#include "DataModel/Design/MemeryMgr.h"
#include "DataModel/Liberty/MemeryMgr.h"
#include "DataModel/Type.h"

namespace db {
class Object {
    friend DesignMemoryMgr;
    friend LibMemeryMgr;

public:
    Object()          = default;
    virtual ~Object() = default;

    DMObjectType        GetType() const;
    virtual std::string PathName();
    virtual size_t      GetSize() const;
    uint64_t            GetObjectFullId() const;
    uint32_t            GetObjectId() const;

private:
    void SetType(DMObjectType type);
    void SetObjectId(uint32_t idx);

private:
    DMObjectType m_type;
    uint32_t     m_obj_id{};
    uint32_t     m_name_id{};
};
}  // namespace db
#endif