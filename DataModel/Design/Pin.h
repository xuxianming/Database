#ifndef DATAMODEL_DESIGN_PIN_H_
#define DATAMODEL_DESIGN_PIN_H_
#include "DataModel/Design/Type.h"
#include "DataModel/Object.h"
namespace db {
class Pin : public Object {
public:
    Pin()           = default;
    ~Pin() override = default;
    size_t GetSize() const override;

private:
};

}  // namespace db

#endif