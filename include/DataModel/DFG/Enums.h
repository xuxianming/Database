#ifndef DATAMODEL_DFG_ENUMS_H_
#define DATAMODEL_DFG_ENUMS_H_

#include <cstdint>

namespace db {

enum class DFGGraphType : uint8_t {
    Module = 0,
    Interface,
    Program,
    Package,
    Function,
    Task,
    GenerateBlock,
    Process,
    Assign,
    Control,
    Signal,
    Blackbox,
    Top
};

enum class DFGPrimType : uint8_t {
    Gate = 0,
    Mux,
    Register,
    Latch,
    Memory,
    Cell,
    Macro,
    Const,
    Operator,
    Udp,
    Blackbox
};

// Detailed operator/expression kind carried by DFGPrim. The node itself has no
// type; its DFGPrim link supplies this information.
enum class DFGOperatorType : uint8_t {
    Unknown = 0,
    // Unary
    Plus,
    Minus,
    BitwiseNot,
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    BitwiseNand,
    BitwiseNor,
    BitwiseXnor,
    LogicalNot,
    Preincrement,
    Predecrement,
    Postincrement,
    Postdecrement,
    // Binary arithmetic/bitwise
    Add,
    Subtract,
    Multiply,
    Divide,
    Mod,
    Power,
    BinaryAnd,
    BinaryOr,
    BinaryXor,
    BinaryXnor,
    // Comparison
    Equality,
    Inequality,
    CaseEquality,
    CaseInequality,
    WildcardEquality,
    WildcardInequality,
    GreaterThanEqual,
    GreaterThan,
    LessThanEqual,
    LessThan,
    // Logical/shift
    LogicalAnd,
    LogicalOr,
    LogicalImplication,
    LogicalEquivalence,
    LogicalShiftLeft,
    LogicalShiftRight,
    ArithmeticShiftLeft,
    ArithmeticShiftRight,
    // Other expression forms that need a distinct operation kind
    Conditional,
    Concatenation,
    Replication,
    Streaming,
    ElementSelect,
    RangeSelect,
    MemberAccess,
    Call,
    Conversion,
    Assignment,
    Inside,
    AssignmentPattern,
    ValueRange,
    MinTypMax,
    TaggedUnion,
    Other
};

enum class DFGLinkKind : uint8_t { Graph = 0, Prim };

enum class DFGPinDirection : uint8_t { Input = 0, Output, InOut, Internal };

enum class DFGEdgeKind : uint8_t { Data = 0, Clock, Reset, Control, Assign, Alias, Unknown };

}  // namespace db

#endif  // DATAMODEL_DFG_ENUMS_H_
