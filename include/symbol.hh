#ifndef SYMBOL_HH
#define SYMBOL_HH

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include "ast.hh"


// Basic symbol table, just keeping track of prior existence and nothing else
struct SymbolTable {
    std::vector<std::map<std::string,std::string>> table;

    SymbolTable(){
        table = std::vector<std::map<std::string,std::string>>(1);
    }

    void push();
    void pop();
    int get_scope();
    void print();
    std::string get_type(std::string key);
    bool scope_contains(std::string key);
    bool contains(std::string key);
    void insert(std::string key, std::string type);
};

#endif