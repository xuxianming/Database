#ifndef DATAMODEL_PORT_H_
#define DATAMODEL_PORT_H_

#include "DataModel/Object.h"
namespace db {
class Port : public Object {
public:
    Port()           = default;
    ~Port() override = default;

private:
    Direction m_dir;
};

}  // namespace db

#endif  // DATAMODEL_PORT_H_