#include <cstdint>
#ifndef DATAMODEL_DESIGN_TYPE_H_
#define DATAMODEL_DESIGN_TYPE_H_

namespace db {
#ifdef _UINT_64_

using PortArrayId     = uint32_t;
using InstArrayId     = uint64_t;
using NetArrayId      = uint64_t;
using PinArrayId      = uint32_t;
using HInstArrayId    = uint32_t;
using HNetArrayId     = uint64_t;
using HPinArrayId     = uint32_t;
using InstBaseArrayId = uint64_t;
using NetBaseArrayId  = uint64_t;
using PinBaseArrayId  = uint32_t;
using BusNetArrayId   = uint32_t;
using BusPortArrayId  = uint32_t;
#else
using PortArrayId     = uint32_t;
using InstArrayId     = uint32_t;
using NetArrayId      = uint32_t;
using PinArrayId      = uint32_t;
using HInstArrayId    = uint32_t;
using HNetArrayId     = uint32_t;
using HPinArrayId     = uint32_t;
using InstBaseArrayId = uint32_t;
using NetBaseArrayId  = uint32_t;
using PinBaseArrayId  = uint32_t;
using BusNetArrayId   = uint32_t;
using BusPortArrayId  = uint32_t;
#endif

}  // namespace db

#endif