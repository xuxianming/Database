#ifndef DATAMODEL_DESIGN_INST_H_
#define DATAMODEL_DESIGN_INST_H_

#include "DataModel/Design/InstBase.h"
namespace db {
class Inst : public InstBase {
public:
    Inst()  =default;
    ~Inst() override = default;
    size_t GetSize() const override;

private:
};

}  // namespace db

#endif