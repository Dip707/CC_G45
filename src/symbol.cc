#include "symbol.hh"

bool SymbolTable::contains(std::string key) {
    int sz = table.size();
    for(int i = sz-1; i >= 0; i--){
        if(table[i].find(key) != table[i].end()){
            return true;
        }
    }
    return false;
}

bool SymbolTable::scope_contains(std::string key) {
    return table.back().find(key) != table.back().end();
}

void SymbolTable::insert(std::string key) {
    table.back().insert(key);
}

void SymbolTable::push() {
    table.push_back(std::set<std::string>());
}

void SymbolTable::pop() {
    table.pop_back();
}