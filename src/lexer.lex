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

"main"                  { return token_main; }
"seq"                   { return token_seq; }
"while"                 { return token_while; }
"array"                 { return token_array; }
"int"                   { return token_int_name; }
"double"                { return token_double_name; }
"function"              { return token_function; }
"print"                 { return token_print; }
"else"                  { return token_else; }
"if"                    { return token_if; }
"for"                   { return token_for; }
"in"                    { return token_in; }
"return"                { return token_return; }
"and"                   { return token_and; }
"or"                    { return token_or; }
"<-"                    { return token_assign; }
"="                     { return token_assign; }
"<="                    { return token_leq; }
"=="                    { return token_eq; }
"!="                    { return token_neq; }
[a-zA-Z]+               { yylval.s = new string(yytext); return token_id; }
0|((-)?[1-9][0-9]*)     { yylval.i = atoi(yytext); return token_int; }
(-)?[0-9]+[.][0-9]+     { yylval.d = atof(yytext); return token_double; }
[:{}()\[\],/<>+*-]      { return *yytext; }
[ \t\n]                 { }
[#].*                   { }
.                       { cout << "Error" << endl; exit(1); }

%%
