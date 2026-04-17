#ifndef DATABASE_OBJECT_H_
#define DATABASE_OBJECT_H_
#include <cstdint>
#include <string>
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Liberty/MemeryMgr.h"
#include "DataModel/Type.h"

namespace db {
class Object {
    friend DesignMemoryMgr;
    friend LibMemeryMgr;

public:
    Object()          = default;
    virtual ~Object() = default;

    DMObjectType             GetType() const;
    virtual std::string_view GetName() const;
    virtual std::string      PathName();
    virtual size_t           GetSize() const;
    FullObjectId             GetObjectFullId() const;
    ObjectId                 GetObjectId() const;
    bool                     IsDesignObject() const;
    bool                     IsLibertyObject() const;

private:
    void SetType(DMObjectType type);

    void SetObjectId(ObjectId idx);

private:
    DMObjectType      m_type;
    ObjectId          m_obj_id{};
    ObjectNameArrayId m_name_id{};
};
}  // namespace db
#endif