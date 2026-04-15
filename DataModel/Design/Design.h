#ifndef DATAMODEL_DESIGN_DESIGN_H_
#define DATAMODEL_DESIGN_DESIGN_H_
#include "DataModel/Design/StringMgr.h"
#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {
class Design : public Object {
    friend class StringMgr<ObjectId>;

public:
    Design()           = default;
    ~Design() override = default;
    size_t   GetSize() const override;
    uint32_t GetMaxFanout() const;

private:
    inline void SetStringMgr(StringMgr<ObjectId>* mgr) { m_string_mgr = mgr; }

private:
    ObjectId             m_top_module_id;
    StringMgr<ObjectId>* m_string_mgr{nullptr};
};

}  // namespace db

#endif