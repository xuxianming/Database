#include "DataModel/Object.h"

namespace db {
DMObjectType Object::GetType() const {
    return static_cast<DMObjectType>((m_obj_id >> 56) & 0xFF);
}
std::string Object::PathName() { return ""; }

void Object::SetType(DMObjectType type) {
    m_obj_id =
        (static_cast<uint64_t>(type) << 56) | (m_obj_id & 0x00FFFFFFFFFFFFFF);
}

uint64_t Object::GetRealId() const { return m_obj_id & 0x00FFFFFFFFFFFFFF; }

void Object::SetRealId(uint64_t id) {
    m_obj_id = (m_obj_id & 0xFF00000000000000) | (id & 0x00FFFFFFFFFFFFFF);
}

uint32_t Object::GetObjectIdx() const {
    return static_cast<uint32_t>(GetRealId() & 0x7F);
}

void Object::SetObjectIdx(uint32_t idx) {
    uint64_t real_id = GetRealId();
    real_id          = (real_id & ~0x7FULL) | (idx & 0x7F);
    SetRealId(real_id);
}

size_t Object::GetSize() const { return sizeof(Object); }
}  // namespace db