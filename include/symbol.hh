#ifndef SYMBOL_HH
#define SYMBOL_HH

#include <set>
#include <vector>
#include <string>
#include "ast.hh"


// Basic symbol table, just keeping track of prior existence and nothing else
struct SymbolTable {
    std::vector<std::set<std::string>> table;

    SymbolTable(){
        table = std::vector<std::set<std::string>>(1);
    }

    void push();
    void pop();
    bool scope_contains(std::string key);
    bool contains(std::string key);
    void insert(std::string key);
};

#endif