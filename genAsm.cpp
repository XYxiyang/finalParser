#include "genAsm.h"
using namespace std;

extern vector<Quaternary>Quas;
vector<string> regs = {
	"$zero", //$0 常量0(constant value 0)
	"$at", //$1 保留给汇编器(Reserved for assembler)
	"$v0","$v1", //$2-$3 函数调用返回值(values for results and expression evaluation)
	"$a0","$a1","$a2","$a3", //$4-$7 函数调用参数(arguments)
	"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7", //$8-$15 暂时的(或随便用的)
	"$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7", //$16-$23 保存的(或如果用，需要SAVE/RESTORE的)(saved)
	"$t8","$t9", //$24-$25 暂时的(或随便用的)
	"$k0","k1",//操作系统／异常处理保留，至少要预留一个 
	"$gp", //$28 全局指针(Global Pointer)
	"$sp", //$29 堆栈指针(Stack Pointer)
	"$fp", //$30 帧指针(Frame Pointer)
	"$ra"//$31 返回地址(return address)
};

GenAsm::GenAsm() {
	for (auto& it : Quas) {
		if (it.retop()[0] == 'j')
			this->Labels.insert(it.retResult());
	}

	for (int i = 0; i < REGNUM; i++) {
		regInfo.push_back({ regs[i],i,0 });
	}

	rvalues[0] = "0";
	for (int i = 1; i < REGNUM; i++) {
		rvalues[i] = "null";
	}
}

void GenAsm::parse() {
	Asms.push_back("addi $sp,$sp," + to_string(STACK_SEG_ADDR));
	Asms.push_back("addi $fp,$fp," + to_string(STACK_SEG_ADDR - 4));

	for (int i = 0; i < Quas.size(); i++) {
		for (auto it = this->regInfo.begin(); it != regInfo.end(); it++) {
			if (it->unuseTime < REG_MAX_UNUSETIME)
				it->unuseTime++;
		}
		this->parseQua(i);
	}
}

void GenAsm::parseQua(int index) {
	Quaternary q = Quas.at(index);
	if (this->Labels.find(to_string(index)) != Labels.end()) {
		Asms.push_back("Label_" + to_string(index) + ":");
	}

	if (q.retop() == "call") {
		for (auto& it : localVariOffsetTable) {
			if (avalues.at(it.first).empty())
				continue;
			//if not exist, no need to save
			int time = REG_MAX_UNUSETIME;
			string regToSave;
			for (auto& r : avalues.at(it.first)) {
				if (regInfo[getRegIndexByName(r)].unuseTime < time) {
					time = regInfo[getRegIndexByName(r)].unuseTime;
					regToSave = r;
				}
			}
			//find latest existence
			Asms.push_back("sw " + regToSave + "," + to_string(it.second) + "($fp)");
			//save vari, by defining offset
		}

		Asms.push_back("jal " + q.retarg1());
		int regIndex = getReg(q.retResult(), -1);

		for (auto& it : localVariOffsetTable) {
			if (avalues.at(it.first).empty())
				continue;
			for (auto& r : avalues.at(it.first)) {
				Asms.push_back("lw " + r + "," + to_string(it.second) + "($fp)");
			}
		}

		Asms.push_back("move " + regs[regIndex] + ",$v1");
		markRegInfo(regIndex);
	}
	else if (q.retop() == "ret") {
		if (q.retarg1() != "_") {
			if (q.retarg1()[0] > '9' || q.retarg1()[0] < '0')//vari
				Asms.push_back("move $v1," + *(avalues.at(q.retarg1()).begin()));
			else //num
				Asms.push_back("addi $v1,$zero" + atoi(q.retarg1().c_str()));
		}

		for (auto it = localVariOffsetTable.begin(); it != localVariOffsetTable.end(); it++) {
			if (avalues.find(it->first) != avalues.end()) {
				for (auto reg = avalues.at(it->first).begin(); reg != avalues.at(it->first).end(); reg++) {
					for (int i = 0; i < REGNUM; i++) {
						if (*reg == regs[i])
							rvalues[i] = "null";
					}
				}
				avalues.erase(it->first);
			}
		}

		Asms.push_back("move $sp,$fp");
		Asms.push_back("addi $sp,$sp," + to_string(paraOffset));
		Asms.push_back("lw $ra,4($fp)");
		Asms.push_back("lw $fp,0($fp)");

		if (procedureName != "main")
			Asms.push_back("jr $ra");

		paraOffset = PARAM_OFFSET_INIT;
		localVariOffset = LOCALVARI_OFFSET_INIT;
		localVariOffsetTable.clear();
		procedureName = "";
	}
	else if (q.retop() == "j") {
		Asms.push_back("j Label_" + q.retResult());
	}
	else if (q.retop() == "+" || q.retop() == "*" || q.retop() == "-" || q.retop() == "/") {
		int index1 = getReg(q.retResult(), -1);
		int index2 = -1;
		int index3 = -1;

		if (avalues.at(q.retarg1()).empty()) {
			if (localVariOffsetTable.find(q.retarg1()) != localVariOffsetTable.end()) {
				int offset = localVariOffsetTable.at(q.retarg1());
				Asms.push_back("lw " + regs[index1] + "," + to_string(offset) + "($fp)");

			}
			else if (globalVariAddrTable.find(q.retarg1()) != globalVariAddrTable.end()) {
				int addr = globalVariAddrTable.at(q.retarg1());
				Asms.push_back("lw " + regs[index1] + "," + to_string(addr) + "($zero)");
			}
			index2 = index1;
		}
		else {
			index2 = getRegIndexByName(*(avalues.at(q.retarg1()).begin()));
		}
		markRegInfo(index2);
		if (!isdigit(q.retarg2()[0])) {
			if (avalues.at(q.retarg2()).empty()) {
				index3 = getReg(q.retarg2(), -1);
			}
			else {
				index3 = getRegIndexByName(*(avalues.at(q.retarg2())).begin());
			}
			markRegInfo(index3);

			if (q.retop() == "+")
				Asms.push_back("add " + regs[index1] + "," + regs[index2] + "," + regs[index3]);
			else if (q.retop() == "-")
				Asms.push_back("sub " + regs[index1] + "," + regs[index2] + "," + regs[index3]);
			else if (q.retop() == "*")
				Asms.push_back("mul " + regs[index1] + "," + regs[index2] + "," + regs[index3]);
			else if (q.retop() == "/") {
				Asms.push_back("div " + regs[index2] + "," + regs[index3]);
				Asms.push_back("mov " + regs[index1] + ",$lo");
			}
		}
		else {
			if (q.retop() == "+")
				Asms.push_back("addi " + regs[index1] + "," + regs[index2] + "," + q.retarg2());
			else if (q.retop() == "-")
				Asms.push_back("subi " + regs[index1] + "," + regs[index2] + "," + q.retarg2());
			else if (q.retop() == "*") {
				Asms.push_back("addi $t8,$zero," + q.retarg2());
				Asms.push_back("mul " + regs[index1] + "," + regs[index2] + ",$t8");

			}
			else if (q.retop() == "/") {
				Asms.push_back("addi $t8,$zero," + q.retarg2());
				Asms.push_back("div " + regs[index2] + ",$t8");
				Asms.push_back("mov " + regs[index1] + ",$lo");
			}
		}
	}
	else if (q.retop() == "=") {
		int index = getReg(q.retResult(), -1);
		if (isdigit(q.retarg1()[0])) {
			Asms.push_back("addi " + regs[index] + ",$zero," + q.retarg1());
			markRegInfo(index);
		}
		else {
			if (!avalues.at(q.retarg1()).empty()) {
				int index2 = getRegIndexByName(*(avalues.at(q.retarg1()).begin()));
				Asms.push_back("move " + regs[index] + "," + regs[index2]);
				markRegInfo(index);
				markRegInfo(index2);
			}
			else {
				if (localVariOffsetTable.find(q.retarg1()) != localVariOffsetTable.end()) {
					int offset = localVariOffsetTable.at(q.retarg1());
					Asms.push_back("lw " + regs[index] + "," + to_string(offset) + "($fp)");

				}
				else if (globalVariAddrTable.find(q.retarg1()) != globalVariAddrTable.end()) {
					int addr = globalVariAddrTable.at(q.retarg1());
					Asms.push_back("lw " + regs[index] + "," + to_string(addr) + "($zero)");
				}
				markRegInfo(index);
			}
		}
	}
	else if (q.retop() == "para") {
		int index;
		if (avalues.at(q.retResult()).empty()) {
			if (localVariOffsetTable.find(q.retResult()) != localVariOffsetTable.end()) {
				int offset = localVariOffsetTable.at(q.retResult());
				index = getReg(q.retResult(), -1);
				Asms.push_back("lw " + regs[index] + "," + to_string(offset) + "($fp)");
				Asms.push_back("subi $sp,$sp,4");
				Asms.push_back("sw " + regs[index] + ",0($sp)");
			}
			else if (globalVariAddrTable.find(q.retResult()) != globalVariAddrTable.end()) {
				int addr = globalVariAddrTable.at(q.retResult());
				index = getReg(q.retResult(), -1);
				Asms.push_back("lw " + regs[index] + "," + to_string(addr) + "($zero)");
				Asms.push_back("subi $sp,$sp,4");
				Asms.push_back("sw " + regs[index] + ",0($sp)");
			}
		}
		else {
			index = getRegIndexByName(*(avalues.at(q.retResult())).begin());
			Asms.push_back("subi $sp,$sp,4");
			Asms.push_back("sw " + regs[index] + ",0($sp)");
		}
		markRegInfo(index);
	}
	else if (q.retop() == "defpara") {
		localVariOffsetTable.insert({ q.retResult(),{paraOffset} });
		paraOffset += 4;
		avalues.insert(pair<string, set<string>>(q.retResult(), {}));
	}
	else if (q.retarg1() == "_" && q.retarg2() == "_" && q.retResult() == "_") {
		procedureName = q.retop();
		localVariOffsetTable.clear();
		paraOffset = PARAM_OFFSET_INIT;
		localVariOffset = LOCALVARI_OFFSET_INIT;

		Asms.push_back(q.retop() + " :");
		Asms.push_back("subi $sp,$sp,4");
		Asms.push_back("sw $ra,0($sp)");
		Asms.push_back("subi $sp,$sp,4");
		Asms.push_back("sw $fp,0($sp)");
		Asms.push_back("move $fp,$sp");
	}
	else if (q.retop() == "j>") {
		int index1 = -1;
		int index2 = -1;
		if (isdigit(q.retarg1()[0])) {
			Asms.push_back("subi $t8,$zero," + q.retarg1());
			index1 = getRegIndexByName("$t8");
		}
		else {
			index1 = getReg(q.retarg1(), -1);
		}

		if (isdigit(q.retarg2()[0])) {
			Asms.push_back("subi $t9,$zero," + q.retarg2());
			index2 = getRegIndexByName("$t9");
		}
		else {
			index2 = getReg(q.retarg2(), -1);
		}

		Asms.push_back("bgt " + regs[index1] + "," + regs[index2] + ",Label_" + q.retResult());
	}
	else if (q.retop() == "j<") {
		int index1 = -1;
		int index2 = -1;
		if (isdigit(q.retarg1()[0])) {
			Asms.push_back("subi $t8,$zero," + q.retarg1());
			index1 = getRegIndexByName("$t8");
		}
		else {
			index1 = getReg(q.retarg1(), -1);
		}

		if (isdigit(q.retarg2()[0])) {
			Asms.push_back("subi $t9,$zero," + q.retarg2());
			index2 = getRegIndexByName("$t9");
		}
		else {
			index2 = getReg(q.retarg2(), -1);
		}

		Asms.push_back("blt" + regs[index1] + "," + regs[index2] + ",Label_" + q.retResult());
	}
	else if (q.retop() == "j>=") {
		int index1 = -1;
		int index2 = -1;
		if (isdigit(q.retarg1()[0])) {
			Asms.push_back("addi $t8,$zero," + q.retarg1());
			index1 = getRegIndexByName("$t8");
		}
		else {
			index1 = getReg(q.retarg1(), -1);
		}

		if (isdigit(q.retarg2()[0])) {
			Asms.push_back("addi $t9,$zero," + q.retarg2());
			index2 = getRegIndexByName("$t9");
		}
		else {
			index2 = getReg(q.retarg2(), -1);
		}

		Asms.push_back("bge " + regs[index1] + "," + regs[index2] + ",Label_" + q.retResult());
	}
	else if (q.retop() == "j<=") {
		int index1 = -1;
		int index2 = -1;
		if (isdigit(q.retarg1()[0])) {
			Asms.push_back("addi $t8,$zero," + q.retarg1());
			index1 = getRegIndexByName("$t8");
		}
		else {
			index1 = getReg(q.retarg1(), -1);
		}

		if (isdigit(q.retarg2()[0])) {
			Asms.push_back("addi $t9,$zero," + q.retarg2());
			index2 = getRegIndexByName("$t9");
		}
		else {
			index2 = getReg(q.retarg2(), -1);
		}

		Asms.push_back("ble " + regs[index1] + "," + regs[index2] + ",Label_" + q.retResult());
	}
	else if (q.retop() == "j=") {
		int index1 = -1;
		int index2 = -1;
		if (isdigit(q.retarg1()[0])) {
			Asms.push_back("subi $t8,$zero," + q.retarg1());
			index1 = getRegIndexByName("$t8");
		}
		else {
			index1 = getReg(q.retarg1(), -1);
		}

		if (isdigit(q.retarg2()[0])) {
			Asms.push_back("subi $t9,$zero," + q.retarg2());
			index2 = getRegIndexByName("$t9");
		}
		else {
			index2 = getReg(q.retarg2(), -1);
		}

		Asms.push_back("beq " + regs[index1] + "," + regs[index2] + ",Label_" + q.retResult());
	}
	else if (q.retop() == "[]=") {
		string offset = q.retResult().substr(q.retResult().find('[') + 1);
		offset.pop_back();
		string arr = q.retResult();//last skipped
		for (int i = arr.length() - 1; i >= 0; i--) {
			if (arr[i] == '[') {
				arr.pop_back();
				break;
			}
			arr.pop_back();
		}
		int indexArr = getReg(arr, -1);
		int indexOffset = getReg(offset, -1);
		int indexAns = getReg(q.retarg1(), -1);
		Asms.push_back("add " + regs[indexOffset] + "," + regs[indexOffset] + "," + regs[indexArr]);
		Asms.push_back("addi " + regs[indexOffset] + "," + regs[indexOffset] + ",4");//addr
		Asms.push_back("sw " + regs[indexAns] + ",(" + regs[indexOffset] + ")");
	}
	else if (q.retop() == "=[]") {
		string offset = q.retarg1().substr(q.retarg1().find('[') + 1);
		offset.pop_back();
		string arr = q.retarg1();//last skipped
		for (int i = arr.length() - 1; i >= 0; i--) {
			if (arr[i] == '[') {
				arr.pop_back();
				break;
			}
			arr.pop_back();
		}
		int indexArr = getReg(arr, -1);
		int indexOffset = getReg(offset, -1);
		int indexAns = getReg(q.retResult(), -1);
		Asms.push_back("add " + regs[indexOffset] + "," + regs[indexOffset] + "," + regs[indexArr]);
		Asms.push_back("addi " + regs[indexOffset] + "," + regs[indexOffset] + ",4");//addr
		Asms.push_back("lw " + regs[indexAns] + ",(" + regs[indexOffset] + ")");
	}
	else if (q.retop() == "[]") {
		localVariOffsetTable.insert(pair<string, int>(q.retarg1(), localVariOffset));
		localVariOffset -= (atoi(q.retResult().c_str()) + 1) * 4;//arr
		Asms.push_back("subi $sp,$sp," + to_string((atoi(q.retResult().c_str()) + 1) * 4));
		avalues.insert({ q.retarg1(),{} });


		int index = getReg(q.retarg1(), -1);//an emoty reg for ptr
		Asms.push_back("add " + regs[index] + ",$zero,$sp");

	}
}

int GenAsm::getReg(string vari, int index) {
	int regIndex = -1;
	if (localVariOffsetTable.find(vari) == localVariOffsetTable.end()
		&& globalVariAddrTable.find(vari) == globalVariAddrTable.end()) {
		if (procedureName == "") {
			globalVariAddrTable.insert({ vari,globalVariAddr });
			globalVariAddr += 4;
		}
		else {
			localVariOffsetTable.insert(pair<string, int>(vari, localVariOffset));
			localVariOffset -= 4;
			Asms.push_back("subi $sp,$sp,4");
		}
		avalues.insert({ vari,{} });
	}

	if (avalues.at(vari).empty()) {//not exist
		for (int i = VARI_REG_START; i <= VARI_REG_END; i++) {
			if (rvalues[i] == "null") {
				regIndex = i;
				avalues.at(vari).insert(regs[regIndex]);
				rvalues[regIndex] = vari;
				break;
			}
		}

		if (regIndex == -1) {
			int forceRegIndex = getLruRegIndex(index);
			string forceRegName = rvalues[forceRegIndex];
			if (avalues.at(forceRegName).size() >= 2) {
				//还在别的寄存器中
			}
			else {
				if (localVariOffsetTable.find(forceRegName) != localVariOffsetTable.end()) {
					int offset = localVariOffsetTable.at(forceRegName);
					Asms.push_back("sw " + regs[forceRegIndex] + "," + to_string(offset) + "($fp)");
				}
				else if (globalVariAddrTable.find(forceRegName) != globalVariAddrTable.end()) {
					int addr = globalVariAddrTable.at(forceRegName);
					Asms.push_back("sw " + regs[forceRegIndex] + "," + to_string(addr) + "($zero)");
				}
			}
			avalues.at(forceRegName).erase(regs[forceRegIndex]);
			regIndex = forceRegIndex;
			rvalues[regIndex] = vari;
			avalues.at(vari).insert(regs[forceRegIndex]);
		}

		if (localVariOffsetTable.find(vari) != localVariOffsetTable.end()) {
			int offset = localVariOffsetTable.at(vari);
			Asms.push_back("lw " + regs[regIndex] + "," + to_string(offset) + "($fp)");
		}
		else if (globalVariAddrTable.find(vari) != globalVariAddrTable.end()) {
			int addr = globalVariAddrTable.at(vari);
			Asms.push_back("lw " + regs[regIndex] + "," + to_string(addr) + "($zero)");
		}
	}
	else {
		string name = *(avalues.at(vari).begin());
		for (int i = 0; i < REGNUM; i++) {
			if (regs[i] == name) {
				regIndex = i;
				break;
			}
		}

	}
	markRegInfo(regIndex);
	return regIndex;
}

int GenAsm::getLruRegIndex(int index) {
	int maxIndex = 0;
	int maxUnuse = 0;
	for (int i = 0; i < regInfo.size(); i++) {
		if (regInfo[i].unuseTime > maxUnuse && regInfo[i].index >= VARI_REG_START
			&& regInfo[i].index <= VARI_REG_END
			&& (index != -1 || index != regInfo[i].index)) {
			//random or specify one reg
			maxIndex = i;
			maxUnuse = regInfo[i].unuseTime;
		}
	}
	return regInfo[maxIndex].index;
}

void GenAsm::markRegInfo(int index) {
	for (auto it = regInfo.begin(); it != regInfo.end(); it++) {
		if (it->index == index) {
			it->unuseTime = 0;
			return;
		}
	}
}

int GenAsm::getRegIndexByName(string name)
{
	for (auto it = regs.begin(); it != regs.end(); it++) {
		if (*it == name)
			return it - regs.begin();
	}
	return -1;
}