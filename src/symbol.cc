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

void SymbolTable::insert(std::string key, std::string type) {
    table.back()[key] = type;
}

void SymbolTable::push() {
    table.push_back(std::map<std::string,std::string>());
}

void SymbolTable::pop() {
    table.pop_back();
}

void SymbolTable::print() {
    int sz = table.size();
    for(int i = sz-1; i >= 0; i--){
        std::cout << "Scope " << i << std::endl;
        for(auto it = table[i].begin(); it != table[i].end(); it++){
            std::cout << it->first << " " << it->second << std::endl;
        }
    }
}

int SymbolTable::get_scope() {
    return table.size();
}

std::string SymbolTable::get_type(std::string key) {
    int sz = table.size();
    for(int i = sz-1; i >= 0; i--){
        if(table[i].find(key) != table[i].end()){
            return table[i][key];
        }
    }
    return "type-error";
}