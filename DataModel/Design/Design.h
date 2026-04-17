#ifndef DATAMODEL_DESIGN_DESIGN_H_
#define DATAMODEL_DESIGN_DESIGN_H_
#include "DataModel/Design/StringMgr.h"
#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {
class ArrayMgr;
class StringMgr<ObjectId>;
class Design : public Object {
public:
    Design()           = default;
    ~Design() override = default;
    size_t   GetSize() const override;
    uint32_t GetMaxFanout() const;
    Module*  GetTopModule() const;
    void     SetTopModule(ObjectId top_module_id);

private:
    inline void SetStringMgr(StringMgr<ObjectId>* mgr) { m_string_mgr = mgr; }
    inline void SetArrayMgr(ArrayMgr* mgr) { m_array_mgr = mgr; }

private:
    ObjectId             m_top_module_id{kInvalidId};
    StringMgr<ObjectId>* m_string_mgr{nullptr};
    ArrayMgr*            m_array_mgr{nullptr};
};

}  // namespace db

#endif