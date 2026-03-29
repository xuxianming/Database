#ifndef DATABASE_TYPE_H_
#define DATABASE_TYPE_H_
#include <cstdint>
#include <string>
namespace db {
#define DMOBJTYPE_ENTRIES                                                      \
  X(OBJECT)                                                                    \
  X(DESIGN)                                                                    \
  X(INST)                                                                      \
  X(NET)                                                                       \
  X(PIN)                                                                       \
  X(PORT)                                                                      \
  X(BUS_PORT)                                                                  \
  X(BUS_NET)                                                                   \
  X(HNET)                                                                      \
  X(HINST)                                                                     \
  X(HPIN)                                                                      \
  X(LIBERTYLIB)                                                                \
  X(LIBCELL)

enum DMObjectType : uint8_t {
#define X(name) name,
  DMOBJTYPE_ENTRIES
#undef X
};
inline std::string GetTypeName(DMObjectType type);
} // namespace db
#endif