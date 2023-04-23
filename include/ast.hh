#ifndef AST_HH
#define AST_HH

#include <llvm/IR/Value.h>
#include <string>
#include <vector>

struct LLVMCompiler;

/**
Base node class. Defined as `abstract`.
*/
struct Node {
    enum NodeType {
        BIN_OP, TYPE_LIT, STMTS, ASSN, DBG, IDENT, TERN, IFELSE
    } type;

    virtual std::string to_string() = 0;
    virtual llvm::Value *llvm_codegen(LLVMCompiler *compiler) = 0;

};

/**
    Node for list of statements
*/
struct NodeStmts : public Node {
    std::vector<Node*> list;

    NodeStmts();
    void push_back(Node *node);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for binary operations
*/
struct NodeBinOp : public Node {
    enum Op {
        PLUS, MINUS, MULT, DIV
    } op;

    Node *left, *right;

    NodeBinOp(Op op, Node *leftptr, Node *rightptr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for literals
*/
struct NodeLIT : public Node {
    struct LIT_value{
        int value_int;
        long value_long;
        short value_short;
    } value;

    enum NodeLIT_Type{
        INT, SHORT, LONG
    } lit_type;

    NodeLIT(NodeLIT_Type lit_type,long value);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};


/**
    Node for variable assignments
*/
struct NodeDecl : public Node {
    std::string identifier;
    std::string datatype;
    Node *expression;

    NodeDecl(std::string id, std::string datatype, Node *expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for `dbg` statements
*/
struct NodeDebug : public Node {
    Node *expression;

    NodeDebug(Node *expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for idnetifiers
*/
struct NodeIdent : public Node {
    std::string identifier;

    NodeIdent(std::string ident);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for variable assignments
*/
struct NodeAssign : public Node {
    std::string identifier;
    Node *expression;

    NodeAssign(std::string id, Node *expr);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

/**
    Node for ternary operation
 */

struct NodeTernary : public Node {
    Node *condition;
    Node *then_expression;
    Node *else_expression;
    
    NodeTernary(Node *expr1, Node *expr2, Node *expr3);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

struct NodeIfElse : public Node {
    Node *expression;
    Node *if_block;
    Node *else_block;

    NodeIfElse(Node *expr, Node *if_b, Node *else_b);
    std::string to_string();
    llvm::Value *llvm_codegen(LLVMCompiler *compiler);
};

#endif