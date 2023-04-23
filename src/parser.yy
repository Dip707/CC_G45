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

int yyerror(std::string msg);

}

%token TPLUS TDASH TSTAR TSLASH
%token <lexeme> TINT_LIT TIDENT TTYPE
%token TLET TDBG 
%token TSCOL TLPAREN TRPAREN TEQUAL
%token TQUES TCOLON 
%token TIF TELSE TBOPEN TBCLOSE

%type <node> Expr Stmt IfElseStmt ScopeOpen ScopeClose
%type <stmts> Program StmtList

%left TEQUAL
%left TQUES TCOLON
%left TPLUS TDASH
%left TSTAR TSLASH

%%

Program :                
        { final_values = nullptr; }
        | StmtList  
        { final_values = $1; }
	    ;

StmtList : 
         { $$ = nullptr; }
         | Stmt
         { $$ = new NodeStmts(); $$->push_back($1); }
	     | StmtList Stmt
         { $$->push_back($2); }
	     ;

Stmt : TLET TIDENT TCOLON TTYPE TEQUAL Expr TSCOL
     {  
        if(symbol_table->scope_contains($2)) {
            // tried to redeclare variable, so error
            yyerror("tried to redeclare variable.\n");
        }else{
            symbol_table->insert($2);
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
     ;

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
        $$ = new NodeIfElse($2, $4, $8);
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
        if(symbol_table->contains($1))
            $$ = new NodeIdent($1); 
        else
            yyerror("using undeclared variable.\n");
     }
     | Expr TPLUS Expr
     {     
        $$ = new NodeBinOp(NodeBinOp::PLUS, $1, $3); 
     }
     | Expr TDASH Expr
     {
        $$ = new NodeBinOp(NodeBinOp::MINUS, $1, $3); 
     }
     | Expr TSTAR Expr
     { 
        $$ = new NodeBinOp(NodeBinOp::MULT, $1, $3); 
     }
     | Expr TSLASH Expr
     { 
        $$ = new NodeBinOp(NodeBinOp::DIV, $1, $3); 
     }
     | TLPAREN Expr TRPAREN 
     { $$ = $2; }
     | Expr TQUES Expr TCOLON Expr
     { $$ = new NodeTernary($1, $3, $5); }
     ;

%%

int yyerror(std::string msg) {
    std::cerr << "Error! " << msg << std::endl;
    exit(1);
}
