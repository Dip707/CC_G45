%define api.value.type { ParserValue }

%code requires {

    #include <iostream>
    #include <bits/stdc++.h>
    using namespace std;
    #include <vector>
    #include <string>

    #include "parser_util.hh"
    #include "symbol.hh"

}

%code {

#include <cstdlib>

extern int yylex();
extern int yyparse();

extern NodeStmts* final_values;

SymbolTable* symbol_table = new SymbolTable();
Folder * folder = new Folder();

int yyerror(std::string msg);

}

%token TPLUS TDASH TSTAR TSLASH
%token <lexeme> TINT_LIT TIDENT TTYPE
%token TLET TDBG 
%token TSCOL TLPAREN TRPAREN TEQUAL
%token TQUES TCOLON 
%token TIF TELSE TBOPEN TBCLOSE
%token TFUN TRETURN TCOMMA

%type <node> Expr Stmt IfElseStmt ScopeOpen ScopeClose FunctionDecl FunctionCall Parameter 
%type <stmts> Program StmtList
%type <parameter_list> ParameterList
%type <arg_list> ArgList

%left TEQUAL
%left TQUES TCOLON
%left TPLUS TDASH
%left TSTAR TSLASH

%%

Program : StmtList
        { final_values = $1; }
	    ;

StmtList : 
         { $$ = new NodeStmts(); }
         | Stmt
         { $$ = new NodeStmts(); $$->push_back($1); }
	     | StmtList Stmt
         { $$->push_back($2); }
	     ;

Stmt : TLET TIDENT TCOLON TTYPE TEQUAL Expr TSCOL
     {  
        if(symbol_table->scope_contains($2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable with same variable or function name.\n");
        }else{
            symbol_table->insert($2,$4);
            $$ = new NodeDecl($2, $4, $6);
        }
     }
     | TDBG Expr TSCOL
     { 
        $$ = new NodeDebug($2);
     } 
     | TIDENT TEQUAL Expr TSCOL
     {
        if(symbol_table->contains($1)) {
            $$ = new NodeAssign($1, $3);
        } else {
            yyerror("using undeclared variable.\n");
        }
     }
     | IfElseStmt
     { $$ = $1;}
     | FunctionDecl
     { $$ = $1;}
     | TRETURN Expr TSCOL 
     { $$ = new NodeReturn($2); }
     ;

Parameter: TIDENT TCOLON TTYPE
    { 
        if(symbol_table->scope_contains($1)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare parameter variable with same variable name.\n");
        }else{
            symbol_table->insert($1,$3);
            $$ = new NodeParameter($1, $3);
        }
    }
    ;

ParameterList: 
        { $$ = new NodeParameterList(); }
        | Parameter
        { $$ = new NodeParameterList();$$->push_back($1); }
        | ParameterList TCOMMA Parameter
        { $$->push_back($3); }
        ;

FunctionDecl : TFUN TIDENT {
        if(symbol_table->get_scope() > 1){
            // tried to declare function in function, so error
            yyerror("Function can only be decalared in global scope.\n");
        }
        if(symbol_table->scope_contains($2)){
            // tried to redeclare variable, so error
            yyerror("tried to redeclare function with same variable or function name.\n");
        }else{
            symbol_table->push();
            symbol_table->insert($2,"function");
        }
    } TLPAREN ParameterList TRPAREN TCOLON TTYPE {
        symbol_table->insert($2,$8);
    } TBOPEN StmtList TBCLOSE {
        // $11->push_back(new NodeReturn(new NodeLIT(NodeLIT::INT,0)));
        $$ = new NodeFunctionDecl($2, $8, $5, $11);
        symbol_table->pop();
    };

ScopeOpen: TBOPEN
    {
        symbol_table->push();
    }
    ;

ScopeClose: TBCLOSE
    { 
        symbol_table->pop();
    }
    ;

IfElseStmt : TIF Expr ScopeOpen StmtList ScopeClose TELSE ScopeOpen StmtList ScopeClose
    {
        NodeIfElse *tmp = new NodeIfElse($2, $4, $8);
        if($2->type == Node::NodeType::TYPE_LIT){
            if($2->to_string() == "0" || stoi($2->to_string()) == 0){
                $$ = tmp->else_block;
            }
            else{
                $$ = tmp->if_block;
            }
        }else
            $$ = tmp;
    }
    ;

Expr : TINT_LIT
     { 
        long long value = stoll($1);
        if(value > INT_MAX || value < INT_MIN) {
            $$ = new NodeLIT(NodeLIT::LONG,value);
        }else{
            $$ = new NodeLIT(NodeLIT::INT,(int)value);
        }
     }
     | TIDENT
     { 
        // symbol_table->print();
        if(symbol_table->contains($1))
            $$ = new NodeIdent($1, symbol_table->get_type($1));
        else
            yyerror("using undeclared variable.\n");
     }
     | Expr TPLUS Expr
     {     
        $$ = folder->fold_constants(new NodeBinOp(NodeBinOp::PLUS, $1, $3)); 
     }
     | Expr TDASH Expr
     {
        $$ = folder->fold_constants(new NodeBinOp(NodeBinOp::MINUS, $1, $3)); 
     }
     | Expr TSTAR Expr
     { 
        $$ = folder->fold_constants(new NodeBinOp(NodeBinOp::MULT, $1, $3)); 
     }
     | Expr TSLASH Expr
     { 
        $$ = folder->fold_constants(new NodeBinOp(NodeBinOp::DIV, $1, $3)); 
     }
     | TLPAREN Expr TRPAREN
     { $$ = $2; }
     | Expr TQUES Expr TCOLON Expr
     { $$ = new NodeTernary($1, $3, $5); }
     | FunctionCall
     { $$ = $1; }
     ;

ArgList: 
        { $$ = new NodeArgList(); }
        | Expr
        { $$ = new NodeArgList(); $$->push_back($1); }
        | ArgList TCOMMA Expr
        { $$->push_back($3); }
        ;

FunctionCall : TIDENT TLPAREN ArgList TRPAREN
    {
        $$ = new NodeFunctionCall($1, $3);
    }
    ;
%%

int yyerror(std::string msg) {
    std::cerr << "Error! " << msg << std::endl;
    exit(1);
}
