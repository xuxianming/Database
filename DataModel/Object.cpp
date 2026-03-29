#include "DataModel/Object.h"
#include <cstdint>

namespace db {
DMObjectType Object::GetType() const { return m_type; }
std::string  Object::PathName() { return ""; }

void Object::SetType(DMObjectType type) { m_type = type; }

uint64_t Object::GetObjectFullId() const {
    return static_cast<uint64_t>(m_obj_id) | static_cast<uint64_t>(m_type);
}

uint32_t Object::GetObjectId() const { return m_obj_id; }

void Object::SetObjectId(uint32_t idx) { m_obj_id = idx; }

size_t Object::GetSize() const { return sizeof(Object); }
}  // namespace db