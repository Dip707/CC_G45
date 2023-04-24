#ifndef PARSER_UTIL_HH
#define PARSER_UTIL_HH

#include <string>
#include <vector>
#include <iostream>

#include "ast.hh"

/**
    Intermediate strcuct used by bison
*/
struct ParserValue {
    std::string lexeme;

    Node *node;
    NodeStmts *stmts;
    NodeArgList *arg_list;
    NodeParameterList *parameter_list;
};


struct Folder {
    Node *fold_constants(Node *node) {
        if (node == nullptr) {
            return nullptr;
        }

        // Recursively fold constants in the children of the current node
        if (NodeStmts *stmts = dynamic_cast<NodeStmts *>(node)) {
            for (size_t i = 0; i < stmts->list.size(); ++i) {
                stmts->list[i] = fold_constants(stmts->list[i]);
            }
        } else if (NodeBinOp *bin_op = dynamic_cast<NodeBinOp *>(node)) {
            bin_op->left = fold_constants(bin_op->left);
            bin_op->right = fold_constants(bin_op->right);

            NodeLIT *left = dynamic_cast<NodeLIT *>(bin_op->left);
            NodeLIT *right = dynamic_cast<NodeLIT *>(bin_op->right);

            if (left && right) {
                long result = 0;
                long lval = 0;
                if(left->gettype() == 16)
                    lval = left->value.value_short;
                else if(left->gettype() == 32)
                    lval = left->value.value_int;
                else
                    lval = left->value.value_long;
                    
                long rval = 0;
                if(right->gettype() == 16)
                    rval = right->value.value_short;
                else if(right->gettype() == 32)
                    rval = right->value.value_int;
                else
                    rval = right->value.value_long;
                      
                switch (bin_op->op) {
                    case NodeBinOp::Op::PLUS:
                        result = lval + rval;
                        break;
                    case NodeBinOp::Op::MINUS:
                        result = lval - rval;
                        break;
                    case NodeBinOp::Op::MULT:
                        result = lval * rval;
                        break;
                    case NodeBinOp::Op::DIV:
                        if (rval == 0) {
                            std::cout << "Error: Division by zero" << std::endl;
                            return nullptr;
                        }
                        result = lval / rval;
                        break;
                }
                std::cout<<result<<"\n";
                return new NodeLIT(NodeLIT::NodeLIT_Type::LONG, result);
            
            
            }
        } else if (NodeTernary *ternary = dynamic_cast<NodeTernary *>(node)) {
            ternary->condition = fold_constants(ternary->condition);
            ternary->then_expression = fold_constants(ternary->then_expression);
            ternary->else_expression = fold_constants(ternary->else_expression);

            NodeLIT *cond = dynamic_cast<NodeLIT *>(ternary->condition);
            if (cond) {
                if(cond->value.value_int){
                    return ternary->then_expression;
                } else {
                    return ternary->else_expression;
                }
            }
        }

        return node;
    }  
};


#endif