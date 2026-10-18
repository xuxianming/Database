#include <slang/ast/ASTVisitor.h>
#include <slang/ast/Compilation.h>
#include <slang/ast/symbols/InstanceSymbols.h>
#include <slang/diagnostics/DiagnosticEngine.h>
#include <slang/diagnostics/Diagnostics.h>
#include <slang/diagnostics/TextDiagnosticClient.h>
#include <slang/syntax/SyntaxTree.h>
#include <iostream>
#include <slang/ast/symbols/PortSymbols.h>

using namespace slang;
using namespace slang::ast;

struct DesignVisitor : public ASTVisitor<DesignVisitor> {
    void Handle(const InstanceSymbol& inst) {
        // 每个顶层实例都会进入这里
        // inst.name 是实例名，inst.body 是
        std::cout << "inst:" << inst.name << std::endl;
    }
    void Handle(const DefinitionSymbol& def) {
        // 每个模块/接口/程序定义都会进入这里
        std::cout << "def:" << def.name << std::endl;
    }
};

int main() {
    // 1. 解析文件：fromFile 返回 TreeOrError（success 为 shared_ptr<SyntaxTree>，failure 为
    // error_code）
    auto result = syntax::SyntaxTree::fromFile("/root/Database/test/test.v");
    if (!result) {
        std::cout << "Failed to load file: " << result.error().second << std::endl;
        return 1;  // 文件加载失败
    }
    auto tree = std::move(*result);

    // 2. 创建 Compilation 并添加语法树
    ast::CompilationOptions options;
    options.topModules.insert("gcd");
    ast::Compilation compilation(options);
    compilation.addSyntaxTree(tree);

    // 3. 获取解析阶段诊断（来自 tree）和 elaboration 阶段诊断（来自 compilation）
    auto parse_diags = tree->diagnostics();
    auto elab_diags  = compilation.getAllDiagnostics();

    if (!parse_diags.empty() || !elab_diags.empty()) {
        DiagnosticEngine diag_engine(tree->sourceManager());
        auto             client = std::make_shared<TextDiagnosticClient>();
        diag_engine.addClient(client);

        for (auto& diag : parse_diags) {
            diag_engine.issue(diag);
        }
        for (auto& diag : elab_diags) {
            diag_engine.issue(diag);
        }

        std::string report = client->getString();
    }

    // 4. 获取细化后的 AST 根（getRoot() 会触发 elaboration 完成）
    const RootSymbol& root = compilation.getRoot();
    std::cout << "root: " << root.name << std::endl;  // 输出 $root
    std::cout << "top instances: " << root.topInstances.size() << std::endl;  // 输出顶层实例数量
    for (const auto* inst : root.topInstances) {
        std::cout << "top instance: " << inst->name << std::endl;  // 输出顶层实例名
        std::cout << "is interface: " << inst->isInterface() << std::endl;  // 输出顶层实例是否是接口
        std::cout << "is module: " << inst->isModule() << std::endl;  // 输出顶层实例是否是模块
        std::cout << "is top level: " << inst->isTopLevel() << std::endl;  // 输出顶层实例是否是顶层
        std::cout << "definition name: " << inst->getDefinition().name << std::endl;  // 输出顶层实例的定义名
        for (const auto *conn : inst->getPortConnections()) {
            std::cout << "\tport connection: " << conn->port.name << std::endl;  // 输出端口连接信息
        }
        std::cout << "port list size: " << inst->body.getPortList().size() << std::endl;  // 输出端口列表大小
        for (const auto *param : inst->body.getPortList()) {
            std::cout << "\t port : " << param->name << std::endl;  // 输出参数信息
            const auto* port_sym = static_cast<const PortSymbol*>(param);
            std::cout << "\t\t port type: " << port_sym->getType().toString() << std::endl;  // 输出端口类型
            std::cout << "\t\t port direction: " << toString(port_sym->direction) << std::endl;  // 输出端口方向
            std::cout << "\t\t port is null: " << port_sym->isNullPort << std::endl;  // 输出端口是否为空
            std::cout << "\t\t port is ansi: " << port_sym->isAnsiPort << std::endl;  // 输出端口是否为 ANSI
            std::cout << "\t\t port is net: " << port_sym->isNetPort() << std::endl;  //
            if (port_sym->getInternalExpr() != nullptr) {
                std::cout << "\t\t port expr kind: " << toString(port_sym->getInternalExpr()->kind) << std::endl;  // 输出端口是否有初始化器
            } else {
                std::cout << "\t\t port has initializer: " << false << std::endl;  // 输出端口是否有初始化器
            }
            // std::cout << "\t\t port expr kind: " << toString(port_sym->getInternalExpr()->kind) << std::endl;  // 输出端口是否有初始化器

        }
    }
    return 0;
}