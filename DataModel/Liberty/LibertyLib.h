#ifndef DATAMODEL_LIBERTY_LIBERTYLIB_H_
#define DATAMODEL_LIBERTY_LIBERTYLIB_H_

#include <cstdint>
#include "DataModel/Liberty/LibertyObject.h"
namespace db {
class LibertyLib : public LibertyObject {
public:
    LibertyLib() = default;
    ~LibertyLib() override = default;
    size_t GetSize() const override;

private:
    uint32_t m_libcells_id;
    uint32_t m_file_name_id;
};
}  // namespace db

#endif