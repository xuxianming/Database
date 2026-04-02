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
    virtual Object*          GetMgrObject() const;

private:
    void SetType(DMObjectType type);

    void SetObjectId(ObjectId idx);

private:
    DMObjectType      m_type;
    ObjectId          m_obj_id{};
    ObjectNameArrayId m_name_id{};
    ObjectNameArrayId m_pathname_id{};
    /**
     * mgr_obj_id is used to store the object ID in the memory manager, which
     * can be used to quickly access the object in the memory manager. It is
     * different from m_obj_id, which is the object ID used in the design. The
     * memory manager may reuse the same object ID for different objects, so
     * mgr_obj_id is used to distinguish between different objects in the memory
     * manager.
     */
    ObjectId mgr_obj_id{};
};
}  // namespace db
#endif