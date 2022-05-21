%{
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include "finalParser.h"
#include "genAsm.h"
using namespace std;
int errorNum=0;
vector<string>items;
vector<Quaternary> Quas;
vector<Function> functions;
vector<string> curFunction;
vector<string> factors;
vector<Array> arrAddr;
vector<int> curIndex;
vector<int> leftIndex;
vector<int> controlIndex;
vector<Quaternary> tempQuas;
vector<int> pass;
vector<vector<string>> paras;
bool isLeft=true;

int T=0;
int index=0;
int yylex(void);
void yyerror(const char *);
extern FILE * yyin;
extern FILE * yyout;
%}

%token INT DOUBLE FLOAT SHORT LONG ID SLBRAC SRBRAC BLBRAC BRBRAC NUM MLBRAC MRBRAC
%token COMMA VOID RETURN ASSIGN WHILE IF ELSE
%token L G EE NE GE LE DELIM
%token MINUS ADD TIME DIVIDE


%%
        Program: sdefine
            ;
        sdefine: define
            | define sdefine
            ;
        define: type ID  defineType
            | VOID ID  {stateFunction(); } defineFunc
            ;
        type: INT
            | LONG 
            | SHORT 
            | DOUBLE 
            | FLOAT 
            ;
        defineType: {cout<<"varis"<<endl;cout<<items.back()<<endl;items.pop_back(); } defineVari
            | {stateFunction(); } defineFunc
            | {cout<<"arrs"<<endl;cout<<items.back()<<endl;items.pop_back(); } defineArr
            ;
        defineVari: DELIM
            ;
        defineFunc: SLBRAC tempParas SRBRAC langField 
            ;
        defineArr: MLBRAC NUM MRBRAC sdefineArr
            ;
        sdefineArr: MLBRAC NUM MRBRAC sdefineArr
            | DELIM
            ;
        tempParas: paraList
            | VOID
            ;
        paraList: para spara
            ;
        spara: COMMA para spara
            |
            ;
        para: type ID {tempParaList();functions.back().stateVari(); }
            ;
        langField: BLBRAC innerState sstring BRBRAC
            ;
        innerState: innerDefineVari
            | 
            ;
        innerDefineVari: INT ID {functions.back().stateVari(); } tarrs DELIM sinnerDefineVari 
            ;
        sinnerDefineVari: INT ID {functions.back().stateVari(); } tarrs DELIM sinnerDefineVari 
            |
            ;
        tarrs: MLBRAC NUM MRBRAC {functions.back().stateArr();} tarrs
            | {functions.back().defineArr();}
            ;
        sstring: string sstring
            | string
            ;
        string: ifString
            | whileString
            | returnString
            | {isLeft=true;}assignString
            ;
        assignString: ID {factors.push_back(items.back());items.pop_back();} ASSIGN {isLeft=false;curIndex.push_back(Quas.size()-1);} expression {assignVari();} DELIM 
            | arr {leftIndex.push_back(T-1);} ASSIGN {isLeft=false;curIndex.push_back(Quas.size()-1);} expression {assignArr();} DELIM
            ; 
        returnString: RETURN {curIndex.push_back(Quas.size()-1);} expression {retur(false);} DELIM
            | RETURN {retur(true);} DELIM
            ;
        whileString: WHILE {controlIndex.push_back(Quas.size()-1); } SLBRAC expression SRBRAC {control();} langField {endControl(true);}
            ;
        ifString: IF {controlIndex.push_back(Quas.size()-1); } SLBRAC expression SRBRAC {control();} langField {endControl(false);} elseField
            ;
        elseField: {startElse();} ELSE langField {endElse();}
            |
            ;
        expression:  {curIndex.push_back(Quas.size()-1);} addExp saddExp  
            ;
        saddExp: relop {curIndex.push_back(Quas.size()-1);} addExp {fillTempQua();} saddExp
            | 
            ;
        relop: L {insertTempQua("j<");}
            | G {insertTempQua("j>");}
            | LE {insertTempQua("j<=");}
            | GE {insertTempQua("j>=");}
            | EE {insertTempQua("j==");}
            | NE {insertTempQua("j!=");}
            ;
        addExp: {curIndex.push_back(Quas.size()-1);} item sitem
            ;
        sitem: ADD {addQua("+");curIndex.push_back(Quas.size()-1);} item {fillQua();}
            | MINUS  {addQua("-");curIndex.push_back(Quas.size()-1);} item {fillQua();}
            |
            ;
        item: {curIndex.push_back(Quas.size()-1);} factor sfactor
            ;
        sfactor: TIME {addQua("*");curIndex.push_back(Quas.size()-1);} factor {fillQua();}
            | DIVIDE {addQua("/");curIndex.push_back(Quas.size()-1);} factor {fillQua();}
            |
            ;
        factor: NUM {factors.push_back(items.back()); items.pop_back();}
            |  SLBRAC expression SRBRAC 
            | ID {factors.push_back(items.back()); items.pop_back();} ftype
            | arr
            ;
        ftype: call 
            |
            ;
        call: SLBRAC {if(pass.size()==0) pass.push_back(0); else pass.push_back(pass.back());} curParas SRBRAC {callFunc();pass.pop_back(); }
            ;
        arr: ID  {factors.push_back(items.back());arrAddr.push_back(items.back());items.pop_back(); arrAddr.back().last=Quas.size()-1; } MLBRAC expression MRBRAC {functions.back().calMidArr();}  sarr
            
        sarr: {arrAddr.back().last=Quas.size()-1;}MLBRAC expression MRBRAC {functions.back().calMidArr();} sarr
            | {functions.back().calArr();}
            ;
        curParas: curParaList
            |
            ;
        curParaList: {curIndex.push_back(Quas.size()-1); paras.push_back({}); } expression {savePara();} sexpression
            ;
        sexpression: COMMA {curIndex.push_back(Quas.size()-1);} expression {savePara();} sexpression
            | {sendPara();}
            ;
%%
void yyerror(const char *str){
    errorNum=1;
    printf("grammar analyze error:%s\n",str);
     
}

int main(int argc,char** argv)
{
    if ((yyin = fopen(argv[1], "r")) == NULL){
	    printf("Can't open file %s\n", argv[1]);
	    return 1;
	}
    curFunction.push_back("main");
    yyparse();
    int i=0;
    /*printf("       - - - - - - - - - - - - - - - - -\n");
    printf("       | op    | arg1  | arg2  | adr   |\n");
    printf("       - - - - - - - - - - - - - - - - -\n");*/
    cout<<endl;
    for(int i=0;i<functions.size();i++){
        cout<<functions.at(i).funcName<<endl;
        for(int j=0;j<functions.at(i).paras.size();j++){
            if(functions.at(i).paras.at(j).isArr)
                cout<<functions.at(i).paras.at(j).variName<<"[]"<<' ';
            else
                cout<<functions.at(i).paras.at(j).variName<<' ';
        }
        cout<<endl;
    }
    for(int i=0;i<Quas.size();i++){
        if(Quas.at(i).retop()=="main"){
            Quaternary temp;
            temp.set("op","j");
            temp.set("arg1","_");
            temp.set("arg2","_");
            temp.set("result",to_string(i+1));
            Quas.insert(Quas.begin(),temp);
            break;
        }
        
    }
    for(int i=0;i<Quas.size();i++){
        cout<<i<<' ';
        Quas.at(i).put();
    }

    GenAsm Asm;
    Asm.parse();

    ofstream out;
    out.open("./asm.txt",ios::out);
    for(int i=0;i<Asm.Asms.size();i++){
        out<<Asm.Asms.at(i)<<endl;
    }
    out.close();
    return 0;
}
 