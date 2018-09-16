%option noyywrap noinput nounput

%{

#include <iostream>
#include <string>
#include <vector>
#include "ast.hpp"
using namespace std;

#include "parser.tab.hpp"

void yyerror(const std::string &msg);

%}

%%

"function"              { return token_function; }
"else"                  { return token_else; }
"if"                    { return token_if; }
"for"                   { return token_for; }
"in"                    { return token_in; }
"return"                { return token_return; }
"<-"                    { return token_assign; }
"="                     { return token_assign; }
"<="                    { return token_leq; }
[a-zA-Z]+               { yylval.s = new string(yytext); return token_id; }
[1-9][0-9]*             { yylval.i = atoi(yytext); return token_int; }
[0-9]+[.][0-9]+         { yylval.d = atof(yytext); return token_double; }
[:{}()\[\],/<>+*-]      { return *yytext; }
[ \t\n]                 { }
[#].*                   { }
.                       { cout << "Error" << endl; exit(1); }

%%
