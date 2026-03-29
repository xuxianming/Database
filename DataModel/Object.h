#ifndef DATABASE_OBJECT_H_
#define DATABASE_OBJECT_H_
#include <cstdint>
#include <string>
#include "DataModel/Design/MemeryMgr.h"
#include "DataModel/Type.h"

namespace db {
class Object {
    friend DesignMemoryMgr;

public:
    Object()          = default;
    virtual ~Object() = default;

    DMObjectType        GetType() const;
    virtual std::string PathName();
    virtual size_t      GetSize() const;
    void                SetType(DMObjectType type);
    uint64_t            GetRealId() const;
    void                SetRealId(uint64_t id);
    uint32_t            GetObjectIdx() const;
    void                SetObjectIdx(uint32_t idx);

private:
    uint64_t m_obj_id{};
    uint32_t m_name_id{};
};
}  // namespace db
#endif