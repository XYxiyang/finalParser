%{
/*copy to c file*/
#include <iostream>
#include <stdio.h> 
#include <string.h>
#include <vector>
#include "finalParser.tab.h"
#include "finalParser.h"
using namespace std;
void yyerror(const char *);
int yywrap(void);
#define YY_SKIP_YYWRAP
#define LETTER 2
#define DIGIT 4
#define CALCULATOR 6
#define ENDING 12
#define HEADER 13
#define MACRO 14
#define NAMESPACE 15
#define CONTENT 16


#define ERROR 24

#define INCLUDE 67
#define CONST 68
#define FOR 73

int yywrap(void){
	return 1;
}
%}

/*regular expresion part*/
space		[ \t \n]
ws {space}+
letter	[A-Za-z_]
id		{letter}({letter}|{digit})*
digit		[0-9]
number	{digit}+(\.{digit}+)?(E[+-]?{digit}+)?
calculator [\+|-|\*|\/|\<|\>]

%s COMMENT
%s COMMENT2
%%

<INITIAL>"/*"						{BEGIN COMMENT;}
<COMMENT>"*/"						{BEGIN INITIAL;}
<COMMENT>.|\n						{;}
<INITIAL>"//"						{BEGIN COMMENT2;}
<COMMENT2>\n					{BEGIN INITIAL;}
<COMMENT2>.					{;}
<INITIAL>{ws} {;}
<INITIAL>if return IF;
<INITIAL>int  return INT;
<INITIAL>void return VOID;
<INITIAL>else return ELSE;
<INITIAL>while return WHILE;
<INITIAL>return return RETURN;
<INITIAL>const return CONST;
<INITIAL>double return DOUBLE;
<INITIAL>float return FLOAT;
<INITIAL>short return SHORT;
<INITIAL>long return LONG;
<INITIAL>for return FOR;
<INITIAL>#include.* return HEADER;
<INITIAL>#define.* return MACRO;
<INITIAL>"#" return ENDING;
<INITIAL>.*:: return NAMESPACE;
<INITIAL>".*" return CONTENT;
<INITIAL>{id} items.push_back(yytext);return ID;
<INITIAL>{number} items.push_back(yytext); return NUM;
<INITIAL>\+  return ADD;
<INITIAL>-  return MINUS;
<INITIAL>\/ return DIVIDE;
<INITIAL>\*  return TIME;
<INITIAL>\<  return L;
<INITIAL>\>  return G;
<INITIAL>\[  return MLBRAC; 
<INITIAL>\]  return MRBRAC; 
<INITIAL>{calculator} return CALCULATOR;
<INITIAL>== return EE;
<INITIAL>>= return GE;
<INITIAL><= return LE;
<INITIAL>!=  return NE;
<INITIAL>=  return ASSIGN;
<INITIAL>;  return DELIM;
<INITIAL>, return COMMA;
<INITIAL>"("  return SLBRAC; 
<INITIAL>")"  return SRBRAC;
<INITIAL>"\{"  return BLBRAC;
<INITIAL>"\}" return BRBRAC;
<INITIAL>. return ERROR;

%%
/*other functions*/
