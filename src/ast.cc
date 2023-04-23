#include "ast.hh"

#include <string>
#include <vector>



NodeBinOp::NodeBinOp(NodeBinOp::Op ope, Node *leftptr, Node *rightptr) {
    type = BIN_OP;
    op = ope;
    left = leftptr;
    right = rightptr;
}



std::string NodeBinOp::to_string() {
    std::string out = "(";
    switch(op) {
        case PLUS: out += '+'; break;
        case MINUS: out += '-'; break;
        case MULT: out += '*'; break;
        case DIV: out += '/'; break;
    }

    out += ' ' + left->to_string() + ' ' + right->to_string() + ')';

    return out;
}

NodeLIT::NodeLIT(NodeLIT::NodeLIT_Type type_lit,long val) {
    type = TYPE_LIT;
    switch(type_lit){
        case INT: 
            lit_type = INT;
            value.value_int = val;
            break;
        case SHORT: 
            lit_type = SHORT;
            value.value_short = val; 
            break;
        case LONG: 
            lit_type = LONG; 
            value.value_long = val;
            break;
    }
}

std::string NodeLIT::to_string() {
    switch(lit_type){
        case INT: return std::to_string(value.value_int);
        case SHORT: return std::to_string(value.value_short);
        case LONG: return std::to_string(value.value_long);
    }
}

NodeStmts::NodeStmts() {
    type = STMTS;
    list = std::vector<Node*>();
}

void NodeStmts::push_back(Node *node) {
    list.push_back(node);
}


std::string NodeStmts::to_string() {
    std::string out = "(begin";
    for(auto i : list) {
        out += " " + i->to_string();
    }

    out += ')';

    return out;
}

NodeDecl::NodeDecl(std::string id,  std::string dt, Node *expr) {
    type = ASSN;
    identifier = id;
    expression = expr;
    datatype = dt;
}

std::string NodeDecl::to_string() {
    return "(let (" + identifier + " " + datatype + ") " + expression->to_string() + ")";
}

NodeDebug::NodeDebug(Node *expr) {
    type = DBG;
    expression = expr;
}

std::string NodeDebug::to_string() {
    return "(dbg " + expression->to_string() + ")";
}

NodeIdent::NodeIdent(std::string ident) {
    identifier = ident;
}
std::string NodeIdent::to_string() {
    return identifier;
}

NodeAssign::NodeAssign(std::string id, Node *expr) {
    type = ASSN;
    identifier = id;
    expression = expr;
}

std::string NodeAssign::to_string() {
    return "(assign " + identifier + " " + expression->to_string() + ")";
}

NodeTernary::NodeTernary(Node *expr1, Node *expr2, Node *expr3){
    type = TERN;
    condition = expr1;
    then_expression = expr2;
    else_expression = expr3;
}

std::string NodeTernary::to_string(){
    return "(?: " + condition->to_string() + " " + then_expression->to_string() + " " + else_expression->to_string() + ")";
}

NodeIfElse::NodeIfElse(Node *expr1, Node *stmt1, Node *stmt2){
    type = IFELSE; 
    expression = expr1;
    if_block = stmt1;
    else_block = stmt2;
}

std::string NodeIfElse::to_string(){
    return "( if-else " + expression->to_string() + "\n" + if_block->to_string() + "\n" + else_block->to_string() + "\n)";
}