#ifndef DATAMODEL_DFG_MEMORYMGR_H_
#define DATAMODEL_DFG_MEMORYMGR_H_

#include <cstdint>

#include "DataModel/DFG/Enums.h"
#include "DataModel/DFG/Type.h"

namespace db {

class DFGEdge;
class DFGGraph;
class DFGNode;
class DFGPin;
class DFGPort;
class DFGPrim;

// DFG object lifetime and table management. DFG tables participate in the
// DesignMemoryMgr snapshot alongside the other DataModel tables.
class DFGMemoryMgr {
public:
    static DFGGraph* CreateGraph(DFGGraphType type);
    static DFGPrim*  CreatePrim(DFGPrimType type,
                                DFGOperatorType op = DFGOperatorType::Unknown);
    static DFGNode*  CreateNode();
    static DFGPort*  CreatePort(DFGPinDirection direction, const DFGType& type);
    static DFGPin*   CreatePin(DFGPinDirection direction, const DFGType& type);
    static DFGEdge*  CreateEdge(DFGEdgeKind kind, const DFGType& type);

    static DFGGraph* GetGraph(ObjectId id);
    static DFGPrim*  GetPrim(ObjectId id);
    static DFGNode*  GetNode(ObjectId id);
    static DFGPort*  GetPort(ObjectId id);
    static DFGPin*   GetPin(ObjectId id);
    static DFGEdge*  GetEdge(ObjectId id);

    static void DestroyGraph(ObjectId id);
    static void DestroyPrim(ObjectId id);
    static void DestroyNode(ObjectId id);
    static void DestroyPort(ObjectId id);
    static void DestroyPin(ObjectId id);
    static void DestroyEdge(ObjectId id);

    // Destroys all nodes, pins, edges and the graph itself. Used when a Design
    // is destroyed or detached.
    static void DestroyGraphTree(ObjectId graph_id);

    static bool SaveAll();
    static bool LoadAll();
    static void Clear();
    static void PrintStats();

private:
    static void UnlinkGraphNode(DFGGraph* graph, ObjectId node_id);
    static void UnlinkGraphEdge(DFGGraph* graph, ObjectId edge_id);
    static void UnlinkGraphPort(DFGGraph* graph, ObjectId port_id);
    static void UnlinkEdgeFromPins(DFGEdge* edge);
};

}  // namespace db

#endif  // DATAMODEL_DFG_MEMORYMGR_H_
