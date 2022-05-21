#pragma once
#include "finalParser.h"
#include <map>
#include <set>
#include <fstream>
using namespace std;

#define REGNUM 32
#define DATA_SEG_ADDR 0x10010000
#define STACK_SEG_ADDR 0x10040000
#define PARAM_OFFSET_INIT 8
#define LOCALVARI_OFFSET_INIT -4
#define REG_MAX_UNUSETIME INT_MAX
#define VARI_REG_START 8
#define VARI_REG_END 15

extern vector<Quaternary> Quas;

struct Reg {
	string name;
	int index;
	int unuseTime;
	bool operator < (const Reg& b) {
		return this->unuseTime < b.unuseTime;
	}
};

class GenAsm {
public:
	string rvalues[REGNUM];
	map<string, set<string>> avalues;
	vector<Reg> regInfo;
	map<string, int>localVariOffsetTable;
	map<string, int>globalVariAddrTable;
	int paraOffset = PARAM_OFFSET_INIT;//true para
	int localVariOffset = LOCALVARI_OFFSET_INIT;//local vari
	int globalVariAddr = DATA_SEG_ADDR;//global vari
	string procedureName = "";
	set<string> Labels;
	vector<string> Asms;

	GenAsm();
	void parse();
	void parseQua(int index);
	int getReg(string vari, int index);
	int getLruRegIndex(int index);
	void markRegInfo(int index);
	int getRegIndexByName(string name);
};