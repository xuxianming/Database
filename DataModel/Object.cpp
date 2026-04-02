#include "DataModel/Object.h"
#include <cstdint>
#include "Object.h"

namespace db {
DMObjectType Object::GetType() const { return m_type; }
std::string  Object::PathName() { return ""; }

void Object::SetType(DMObjectType type) { m_type = type; }

std::string_view Object::GetName() const { return ""; }

FullObjectId Object::GetObjectFullId() const {
    return (static_cast<FullObjectId>(m_type) << TypeOffset) | m_obj_id;
}

ObjectId Object::GetObjectId() const { return m_obj_id; }

void Object::SetObjectId(ObjectId idx) { m_obj_id = idx; }

bool Object::IsDesignObject() const {
    return m_type < DMObjectType::DESIGNOBJ_COUNT;
}

bool Object::IsLibertyObject() const {
    return m_type > DMObjectType::DESIGNOBJ_COUNT &&
           m_type < DMObjectType::LIBOBJECT_COUNT;
}

Object* Object::GetMgrObject() const { return nullptr; }

size_t Object::GetSize() const { return sizeof(Object); }
}  // namespace db