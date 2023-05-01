#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <fstream>

#include "parser.hh"
#include "ast.hh"
#include "llvmcodegen.hh"

extern FILE *yyin;
extern int yylex();
extern char *yytext;

NodeStmts *final_values;

#define ARG_OPTION_L 0
#define ARG_OPTION_P 1
#define ARG_OPTION_S 2
#define ARG_OPTION_O 3
#define ARG_FAIL -1

// Node *fold_constants(Node *node) {
//     if (node == nullptr) {
//         return nullptr;
//     }

//     // Recursively fold constants in the children of the current node
//     if (NodeStmts *stmts = dynamic_cast<NodeStmts *>(node)) {
//         for (size_t i = 0; i < stmts->list.size(); ++i) {
//             stmts->list[i] = fold_constants(stmts->list[i]);
//         }
//     } else if (NodeBinOp *bin_op = dynamic_cast<NodeBinOp *>(node)) {
//         bin_op->left = fold_constants(bin_op->left);
//         bin_op->right = fold_constants(bin_op->right);

//         NodeLIT *left = dynamic_cast<NodeLIT *>(bin_op->left);
//         NodeLIT *right = dynamic_cast<NodeLIT *>(bin_op->right);

//         if (left && right && left->lit_type == right->lit_type) {
//             long result = 0;
//             switch (bin_op->op) {
//                 case NodeBinOp::Op::PLUS:
//                     result = left->value.value_int + right->value.value_int;
//                     break;
//                 case NodeBinOp::Op::MINUS:
//                     result = left->value.value_int - right->value.value_int;
//                     break;
//                 case NodeBinOp::Op::MULT:
//                     result = left->value.value_int * right->value.value_int;
//                     break;
//                 case NodeBinOp::Op::DIV:
//                     if (right->value.value_int == 0) {
//                         std::cout << "Error: Division by zero" << std::endl;
//                         return nullptr;
//                     }
//                     result = left->value.value_int / right->value.value_int;
//                     break;
//             }
//             return new NodeLIT(left->lit_type, result);
//         }
//     } else if (NodeTernary *ternary = dynamic_cast<NodeTernary *>(node)) {
//         ternary->expression1 = fold_constants(ternary->expression1);
//         ternary->expression2 = fold_constants(ternary->expression2);
//         ternary->expression3 = fold_constants(ternary->expression3);

//         NodeLIT *cond = dynamic_cast<NodeLIT *>(ternary->expression1);
//         if (cond) {
//             if(cond->value.value_int){
//                 return ternary->expression2;
//             } else {
//                 return ternary->expression3;
//             }
//         }
//     }

//     return node;
// }


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
                    rval = left->value.value_short;
                else if(right->gettype() == 32)
                    rval = left->value.value_int;
                else
                    rval = left->value.value_long;
                    
                    
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

        return static_cast<Node*>(node);
}  

int parse_arguments(int argc, char *argv[]) {
	if (argc == 3 || argc == 4) {
		if (strlen(argv[2]) == 2 && argv[2][0] == '-') {
			if (argc == 3) {
				switch (argv[2][1]) {
					case 'l':
					return ARG_OPTION_L;

					case 'p':
					return ARG_OPTION_P;

					case 's':
					return ARG_OPTION_S;
				}
			} else if (argv[2][1] == 'o') {
				return ARG_OPTION_O;
			}
		}
	} 
	
	std::cerr << "Usage:\nEach of the following options halts the compilation process at the corresponding stage and prints the intermediate output:\n\n";
	std::cerr << "\t`./bin/base <file_name> -l`, to tokenize the input and print the token stream to stdout\n";
	std::cerr << "\t`./bin/base <file_name> -p`, to parse the input and print the abstract syntax tree (AST) to stdout\n";
	std::cerr << "\t`./bin/base <file_name> -s`, to compile the file to LLVM assembly and print it to stdout\n";
	std::cerr << "\t`./bin/base <file_name> -o <output>`, to compile the file to LLVM bitcode and write to <output>\n";
	return ARG_FAIL;
}


int main(int argc, char *argv[]) {
	int arg_option = parse_arguments(argc, argv);
	if (arg_option == ARG_FAIL) {
		exit(1);
	}

	std::string file_name(argv[1]);
	FILE *source = fopen(argv[1], "r");

    if(!source) {
        std::cerr << "File does not exists.\n";
        exit(1);
    }
	yyin = source;

	if (arg_option == ARG_OPTION_L) {
		extern std::string token_to_string(int token, const char *lexeme);

		while (true) {
			int token = yylex();
			if (token == 0) {
				break;
			}
			std::cout << token_to_string(token, yytext) << "\n";
		}
		fclose(yyin);
		return 0;
	}

    final_values = nullptr;
	yyparse();

	fclose(yyin);

	//implement control folding
	// final_values = fold_constants(final_values);

	if(final_values) {

		// here
        string myStr = final_values->to_string();
        FILE* file = fopen ("opt.txt", "w");
        fwrite(myStr.data(), sizeof(char), myStr.size(), file);
        fclose(file);

		if (arg_option == ARG_OPTION_P) {
            cout<<final_values->to_string()<<endl;
			return 0;
		}
		
        llvm::LLVMContext context;
		LLVMCompiler compiler(&context, "base");
		compiler.compile(final_values);
        if (arg_option == ARG_OPTION_S) {
			compiler.dump();
        } else {
            compiler.write(std::string(argv[3]));
		}
	} else {
	 	std::cerr << "empty program\n";
		exit(1);
	}

    return 0;
}