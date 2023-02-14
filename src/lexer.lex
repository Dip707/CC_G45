%option noyywrap
 
%{
#include "parser.hh"
#include <map>
#include <string>
#include <unordered_set>
using namespace std;
 
map <string, string> macro_map;
 
extern int yyerror(std::string msg);

string getvalue(string m_key) {
    unordered_set<string> vis_mac;
    vis_mac.clear();
    
    for(;macro_map.count(m_key) > 0 && macro_map[m_key] != "";m_key = macro_map[m_key]) {
        if (vis_mac.count(m_key) > 0) {
            yyerror("Define marco again : Circular dependancy detected");
            return 0;
        }
        
        vis_mac.insert(m_key);
    }
    
    return m_key;
}
%}
 
%%
 
"#def "     {
    int ch;
    int flag = 0; 
    string val = "";
    string m_key = "";
    
    for(;(ch = yyinput()) != 0;) {
        if (flag == 0) {
            if(ch == '\n')
                break;
            else if(ch == ' ')
            {
                flag = 1;
                continue;
            }
            else
                m_key += ch;
        } 
        else if (flag == 1) { 
            if (ch == '\\') { 
                flag = 2;
                continue;
            }
            else if (ch == '\n') { 
                break;
            }
        } 
        else if (flag == 2) { 
            flag = 1;
        }
        if(flag != 0)
            val += ch;
    }
    
    if (val == "")
        macro_map[m_key] = "1";
    else 
        macro_map[m_key] = val;
}
"#undef "   {
    int ch; 
    string m_key = "";
    
    // read m_key
    for(;(ch = yyinput()) != 0 && ch != '\n';) 
        m_key += ch; 
            
    macro_map[m_key] = "";
}
 
"+"       { return TPLUS; }
"-"       { return TDASH; }
"*"       { return TSTAR; }
"/"       { return TSLASH; }
";"       { return TSCOL; }
"("       { return TLPAREN; }
")"       { return TRPAREN; }
"="       { return TEQUAL; }
"dbg"     { return TDBG; }
"let"     { return TLET; }
[0-9]+    { yylval.lexeme = string(yytext); return TINT_LIT; }
[a-zA-Z]+ {
    
    string sub = getvalue(string(yytext));
    
    if(sub == string(yytext)) { 
        yylval.lexeme = sub;
        return TIDENT;
    }
    else {
        if(sub.size() <= strlen(yytext)) { 
            int len = sub.size() - 1;
            for (int i = strlen(yytext) - 1; i > -1; --i) {
                if(i <= len)
                    unput(sub[i]);
                else 
                    unput(' ');
            }
        }
        else {
            for (int i = sub.size() - 1; i > -1; --i) {
                unput(sub[i]);
            }
        }
    } 
}
[ \t\n]   { /* skip */ }
"//".*      { /*skip*/ }
"/*"[a-zA-Z0-9 \n]*"*/" {/* skip */}
.         { yyerror("unknown char"); }
 
%%
 
std::string token_to_string(int token, const char *lexeme) {
    std::string s;
    switch (token) {
        case TPLUS: s = "TPLUS"; break;
        case TDASH: s = "TDASH"; break;
        case TSTAR: s = "TSTAR"; break;
        case TSLASH: s = "TSLASH"; break;
        case TSCOL: s = "TSCOL"; break;
        case TLPAREN: s = "TLPAREN"; break;
        case TRPAREN: s = "TRPAREN"; break;
        case TEQUAL: s = "TEQUAL"; break;
        
        case TDBG: s = "TDBG"; break;
        case TLET: s = "TLET"; break;
        
        case TINT_LIT: s = "TINT_LIT"; s.append("  ").append(lexeme); break;
        case TIDENT: s = "TIDENT"; s.append("  ").append(lexeme); break;
    }
    
    
    return s;
}