#ifndef DATAMODEL_DESIGN_PIN_H_
#define DATAMODEL_DESIGN_PIN_H_
#include "DataModel/Design/PinBase.h"
namespace db {
class Pin : public PinBase {
public:
    Pin() = default;
    ~Pin() override = default;
    size_t GetSize() const override;

private:
};

}  // namespace db

#endif