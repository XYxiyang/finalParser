#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <iomanip>
using namespace std;
extern int errorNum;
extern int T;
extern int index;


struct Vari {
    string variName;
    bool isArr;
    vector<int> dims;
};

class Array {
public:
    string name;
    int dim;
    vector<int> index;
    int last;
    Array(string n) {
        this->name = n;
        dim = 0;
        last = 0;
    }
};

class Quaternary {
    int index;
    string op;
    string arg1;
    string arg2;
    string result;
public:
    Quaternary() { index = 0; }
    void set(string pos, string cont);
    void put();
    string retop() {
        return this->op;
    }
    string retarg1(){
        return this->arg1;
    }
    string retarg2() {
        return this->arg2;
    }
    string retResult() {
        return this->result;
    }

};

class Function {
    
public:
    int paraListNum;
    vector<Vari> paras;
    string funcName;
    Function(string name) { paraListNum = 0; funcName = name; }
    void stateVari();
    void stateArr();
    void calArr();
    int getPara(string name);
    void calMidArr();
};

extern vector<string> items;
extern vector<Quaternary> Quas;
extern vector<Function> functions;
extern vector<string> curFunction;
extern vector<string> factors;
extern vector<Array> arrAdd;
extern vector<int> curIndex;
extern vector<int> leftIndex;
extern vector<Quaternary> tempQuas;
extern vector<int> controlIndex;
extern bool isLeft;

void stateFunction();
void addQua(string op);
void fillQua();
void assignArr();
void assignVari();
void retur(bool isEmpty);
void insertTempQua(string op);
void fillTempQua();
void control();
void endControl(bool isWhile);
void callFunc();
void sendPara();
void tempParaList();