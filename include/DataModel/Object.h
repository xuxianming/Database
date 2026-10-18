#ifndef DATABASE_OBJECT_H_
#define DATABASE_OBJECT_H_
#include <string>
#include "DataModel/Design/MemoryMgr.h"
#include "DataModel/Enums.h"
#include "DataModel/Liberty/MemoryMgr.h"

namespace db {
class Object {
    friend DesignMemoryMgr;
    friend LibertyMemoryMgr;

public:
    Object()          = default;
    virtual ~Object() = default;

    DMObjectType             GetType() const;
    ObjectNameId             GetNameId() const { return m_name_id; }
    virtual std::string_view GetName() const;
    virtual std::string      PathName();
    virtual size_t           GetSize() const;
    FullObjectId             GetObjectFullId() const;
    inline ObjectId          GetObjectId() const { return m_obj_id; }
    bool                     IsDesignObject() const;
    bool                     IsLibertyObject() const;

private:
    void SetType(DMObjectType type);
    void SetObjectId(ObjectId idx);

protected:
    DMObjectType m_type;
    ObjectId     m_obj_id{};
    ObjectNameId m_name_id{};
};
}  // namespace db
#endif