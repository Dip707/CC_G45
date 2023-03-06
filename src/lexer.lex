%option noyywrap
%{
#include "parser.hh"
#include <bits/stdc++.h>
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
 
"#ifdef " {
    //#ifdef
    string ifdef_var = "#ifdef ";
    //#elif
    string elif_var = "#elif ";
    //#else
    string else_var = "#else";
    //#endif
    string endif_var = "#endif";
    string str = string(yytext);
    
    int pos_ifdef = 0;
    int pos_elif = str.find(elif_var);
    int pos_else = str.find(else_var);
    int pos_endif = str.find(endif_var);
    
    while(true){
        str += yyinput();
        string temp = "";
        for(int i = 0; i < 6; i++)temp += str[str.size() - 1 - i];
        reverse(temp.begin(), temp.end());
        if(temp == endif_var)break;
    }

    //we need to remove #ifdef and add it as #elif
    reverse(str.begin(), str.end());

    for(int i = 0; i < 7; i++)str.pop_back();
    for(int i = 0; i < 6; i++) str += elif_var[elif_var.size() - 1 - i];

    reverse(str.begin(), str.end());
    int n = str.size();

    //"#elif DEBUG \n dkfhdslnkfhsdlksdfjsdfldf #elif DEBUG2 ndsjfsdbfksdjbf \n #else \n diahdjlfjd #endif"
        bool elif_flag = false;
    int pos = -1;
    string ans = "";
    while(true){
        // printf("here I am stuck\n");
        pos = str.find(elif_var, pos + 1);
        // cout<<pos<<"\n";
        if(pos != -1){
            //condition check
            string check = "";
            int end_check = pos;
            for(int i = pos + 6; i < n; i++){
                if(str[i] == ' ' || str[i] == '\n'){
                    break;
                }
                check+=str[i];
                end_check = i;
            }
            
            elif_flag = (macro_map.find(check) != macro_map.end());
            // cout<<check<<" "<<elif_flag<<" checking flag\n";
            // checked
            if(elif_flag)
            {
                int end_pos = pos;
                end_pos = str.find(elif_var, pos+1);
                if(end_pos == -1)
                {
                    end_pos = str.find(else_var, pos+1);
                    if(end_pos == -1)
                    {
                        end_pos = str.find(endif_var, pos+1);
                    }
                }
                for(int i = end_pos-1; i > end_check; i--)
                {
                    // unput(str[i]);
                    ans += str[i];
                }
                break;
            }
            // cout<<check<<" checking"<<"\n";
            
        }
        else
        {
            pos = str.find(else_var);
            if(pos != -1)
            {
                int end_pos = str.find(endif_var, pos+1);
                for(int i = end_pos-1; i >= pos+5; i--)
                {
                    // unput(str[i]);
                    ans += str[i];
                }
            }
            break;
        }
    }
    // cout<<ans<<"\n";
    for(auto ch : ans)
        unput(ch);

        
}
 
"+"       { return TPLUS; }
"-"       { return TDASH; }
"*"       { return TSTAR; }
"/"       { return TSLASH; }
";"       { return TSCOL; }
"("       { return TLPAREN; }
")"       { return TRPAREN; }
"="       { return TEQUAL; }
"?"       {return TQUES;}
":"       {return TCOLON;}
"dbg"     { return TDBG; }
"let"     { return TLET; }
[0-9]+    { yylval.lexeme = string(yytext); return TINT_LIT; }
[a-zA-Z]+ {
    
    string sub = getvalue(string(yytext));
    // cerr<<string(yytext)<<'\n';
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
        case TQUES: s = "TQUES"; break;
        case TCOLON: s = "TCOLON"; break;
    }
    
    
    return s;
}