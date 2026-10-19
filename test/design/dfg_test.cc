// Tests for the DFG object core and Design attachment rule.

#include "DataModel/DFG/DFGEdge.h"
#include "DataModel/DFG/DFGGraph.h"
#include "DataModel/DFG/DFGNode.h"
#include "DataModel/DFG/DFGPin.h"
#include "DataModel/DFG/DFGPrim.h"
#include "DataModel/DFG/DFGPort.h"
#include "DataModel/DFG/MemoryMgr.h"
#include "DataModel/Design/Design.h"
#include "DataModel/Design/MemoryMgr.h"

#include <gtest/gtest.h>

TEST(DfgTest, AttachModuleGraphAndBuildLeaf) {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::DFGEdgeKind;
    using db::DFGGraphType;
    using db::DFGMemoryMgr;
    using db::DFGPinDirection;
    using db::DFGPrimType;
    using db::DFGType;

    Design* design = DesignMemoryMgr::CreateDesign();
    ASSERT_NE(design, nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    auto* module_graph = DFGMemoryMgr::CreateGraph(DFGGraphType::Module);
    ASSERT_NE(module_graph, nullptr);
    EXPECT_TRUE(design->AttachDFG(module_graph));
    EXPECT_EQ(design->GetDFG(), module_graph);

    // Only a Module graph can be mounted on Design.
    auto* interface_graph = DFGMemoryMgr::CreateGraph(DFGGraphType::Interface);
    ASSERT_NE(interface_graph, nullptr);
    EXPECT_FALSE(design->AttachDFG(interface_graph));

    Design* other_design = DesignMemoryMgr::CreateDesign();
    ASSERT_NE(other_design, nullptr);
    EXPECT_FALSE(other_design->AttachDFG(module_graph));
    DesignMemoryMgr::DestroyDesign(other_design->GetObjectId());

    // Primitive definition and primitive instance node.
    auto* prim = DFGMemoryMgr::CreatePrim(DFGPrimType::Operator,
                                          db::DFGOperatorType::Add);
    ASSERT_NE(prim, nullptr);
    EXPECT_EQ(prim->GetPrimType(), DFGPrimType::Operator);
    EXPECT_EQ(prim->GetOperatorType(), db::DFGOperatorType::Add);

    auto* prim_node = DFGMemoryMgr::CreateNode();
    ASSERT_NE(prim_node, nullptr);
    prim_node->SetPrim(prim);
    module_graph->AddNode(prim_node);
    EXPECT_EQ(prim_node->GetLink(), prim);
    EXPECT_EQ(prim_node->GetPrim(), prim);
    EXPECT_EQ(prim_node->GetGraph(), nullptr);

    auto* port = DFGMemoryMgr::CreatePort(DFGPinDirection::Input,
                                          DFGType{8, true, false});
    ASSERT_NE(port, nullptr);
    module_graph->AddPort(port);
    EXPECT_EQ(module_graph->GetPortHead(), port->GetObjectId());

    auto* io_pin = port->GetIoPin();
    ASSERT_NE(io_pin, nullptr);
    EXPECT_EQ(port->GetIoPinId(), io_pin->GetObjectId());
    EXPECT_EQ(io_pin->GetOwnerPortId(), port->GetObjectId());

    auto* a = DFGMemoryMgr::CreatePin(DFGPinDirection::Input, DFGType{1, false, false});
    auto* y = DFGMemoryMgr::CreatePin(DFGPinDirection::Output, DFGType{1, false, false});
    ASSERT_NE(a, nullptr);
    ASSERT_NE(y, nullptr);
    prim_node->AddPin(a);
    prim_node->AddPin(y);

    auto* edge = DFGMemoryMgr::CreateEdge(DFGEdgeKind::Data, DFGType{1, false, false});
    ASSERT_NE(edge, nullptr);
    edge->Connect(a, y);
    module_graph->AddEdge(edge);

    EXPECT_EQ(module_graph->GetGraphType(), DFGGraphType::Module);
    EXPECT_EQ(module_graph->GetNodeHead(), prim_node->GetObjectId());
    EXPECT_EQ(module_graph->GetEdgeHead(), edge->GetObjectId());
    EXPECT_EQ(edge->GetSrcPinId(), a->GetObjectId());
    EXPECT_EQ(edge->GetDstPinId(), y->GetObjectId());
    EXPECT_EQ(edge->GetType().width, 1u);

    const db::ObjectId interface_id = interface_graph->GetObjectId();
    const db::ObjectId module_id    = module_graph->GetObjectId();
    const db::ObjectId design_id    = design->GetObjectId();

    // The attached module graph is destroyed with the Design.
    DFGMemoryMgr::DestroyGraph(interface_id);
    DesignMemoryMgr::DestroyDesign(design_id);
    EXPECT_EQ(DFGMemoryMgr::GetGraph(module_id), nullptr);
    Design::SetCurrentDesign(db::kInvalidId);
}

TEST(DfgTest, PersistenceRoundtrip) {
    using db::Design;
    using db::DesignMemoryMgr;
    using db::DFGGraphType;
    using db::DFGMemoryMgr;
    using db::DFGPinDirection;
    using db::DFGType;

    Design* design = DesignMemoryMgr::CreateDesign();
    ASSERT_NE(design, nullptr);
    Design::SetCurrentDesign(design->GetObjectId());

    auto* graph = DFGMemoryMgr::CreateGraph(DFGGraphType::Module);
    ASSERT_NE(graph, nullptr);
    ASSERT_TRUE(design->AttachDFG(graph));

    auto* node = DFGMemoryMgr::CreateNode();
    ASSERT_NE(node, nullptr);
    node->SetGraph(graph);
    graph->AddNode(node);

    auto* port = DFGMemoryMgr::CreatePort(DFGPinDirection::Output,
                                          DFGType{16, false, false});
    ASSERT_NE(port, nullptr);
    graph->AddPort(port);
    auto* io_pin = port->GetIoPin();
    ASSERT_NE(io_pin, nullptr);

    const db::ObjectId design_id = design->GetObjectId();
    const db::ObjectId graph_id  = graph->GetObjectId();
    const db::ObjectId node_id   = node->GetObjectId();
    const db::ObjectId port_id   = port->GetObjectId();
    const db::ObjectId io_pin_id = io_pin->GetObjectId();

    ASSERT_TRUE(DesignMemoryMgr::SaveAll());
    ASSERT_TRUE(DesignMemoryMgr::LoadAll("."));

    Design* loaded = Design::GetCurrentDesign();
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->GetObjectId(), design_id);
    auto* loaded_graph = loaded->GetDFG();
    ASSERT_NE(loaded_graph, nullptr);
    EXPECT_EQ(loaded_graph->GetObjectId(), graph_id);
    EXPECT_EQ(loaded_graph->GetNodeHead(), node_id);
    EXPECT_EQ(loaded_graph->GetPortHead(), port_id);

    auto* loaded_port = DFGMemoryMgr::GetPort(port_id);
    ASSERT_NE(loaded_port, nullptr);
    EXPECT_EQ(loaded_port->GetIoPinId(), io_pin_id);
    ASSERT_NE(DFGMemoryMgr::GetPin(io_pin_id), nullptr);

    auto* loaded_node = DFGMemoryMgr::GetNode(node_id);
    ASSERT_NE(loaded_node, nullptr);
    EXPECT_EQ(loaded_node->GetLinkId(), graph_id);
    EXPECT_EQ(loaded_node->GetLinkKind(), db::DFGLinkKind::Graph);

    DesignMemoryMgr::DestroyDesign(loaded->GetObjectId());
    Design::SetCurrentDesign(db::kInvalidId);
}
