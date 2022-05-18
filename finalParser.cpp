#include "finalParser.h"
using namespace std;
extern vector<string>items;
extern vector<Quaternary> Quas;
extern vector<Function> functions;
extern vector<string> curFunction;
extern vector<string> factors;
extern vector<Array> arrAddr;
extern vector<int> curIndex;
extern vector<int> leftIndex;
extern vector<Quaternary> tempQuas;
extern vector<int> controlIndex;
extern bool isLeft;
extern int errorNum;
extern int T;
extern int index;
void Function::stateVari() {
	string name = items.back();
	cout << name <<"state"<< endl;
	items.pop_back();
	for (int i = 0; i < this->paras.size(); i++) {
		if (name == this->paras.at(i).variName) {
			errorNum = 1;
			cout << "ERROR::try to state a variable that exists" << endl;
			return;
		}
	}
	Vari temp;
	temp.isArr = false;
	temp.variName = name;
	this->paras.push_back(temp);
}

void Function::stateArr() {
	(this->paras.back().dims).push_back(atoi(items.back().c_str()));
	this->paras.back().isArr = true;
	items.pop_back();
}

void Function::calArr() {
	Quaternary temp;
	temp.set("op", "+");
	temp.set("arg1", Quas.at(arrAddr.back().index.at(0)).retResult());
	temp.set("arg2", factors.back());
	temp.set("result", "T" + to_string(T++));
	Quas.push_back(temp);
	factors.pop_back();

	for (int i = 1; i < arrAddr.back().index.size();i++) {
		temp.set("arg1", Quas.back().retResult());
		temp.set("arg2", Quas.at(arrAddr.back().index.at(i)).retResult());
		temp.set("result", "T" + to_string(T++));
		Quas.push_back(temp);
	}

	temp.set("op", "*");
	temp.set("arg1", Quas.back().retResult());

	temp.set("arg2", "4");
	temp.set("result", string("T") + to_string(T++));
	Quas.push_back(temp);

	temp.set("op", "-");
	temp.set("arg1", arrAddr.back().name);
	Vari arr = this->paras.at(this->getPara(arrAddr.back().name));
	int space = 1;
	for (int i = 0; i < arr.dims.size(); i++) {
		space *= arr.dims.at(i);
	}
	temp.set("arg2", to_string((space + 1) * 4));
	temp.set("result", string("T") + to_string(T++));

	Quas.push_back(temp);

	
	
	if (!isLeft) {
		temp.set("op", "=[]");
		temp.set("arg1", "T" + to_string(T - 1) + "[T" + to_string(T - 2) + "]");
		temp.set("arg2", "_");
		temp.set("result", string("T") + to_string(T++));
		Quas.push_back(temp);
	}
	else
		isLeft = true;
	
	arrAddr.pop_back();

}

int Function::getPara(string name) {
	for (int i = 0; i < this->paras.size(); i++) {
		//cout << "name:" << name << ' ' << paras.at(i).variName << endl;
		if (paras.at(i).variName == name) {
			return i;
		}
	}
	return -1;
}

void Function::calMidArr() {
	Vari arr = this->paras.at(this->getPara(arrAddr.back().name));
	Quaternary temp;
	temp.set("op", "*");
	for (int i = ++arrAddr.back().dim; i < arr.dims.size(); i++) {
		if(arrAddr.back().last!=Quas.size()-1)//num
			temp.set("arg1", "T" + to_string(T - 1));
		else {
			temp.set("arg1", factors.back());//operate
		}
		factors.pop_back();
		temp.set("arg2", to_string(arr.dims.at(i)));
		temp.set("result", "T" + to_string(T++));
		Quas.push_back(temp);
		
	}
	if(arrAddr.back().index.size()==0||Quas.size()-1!= arrAddr.back().index.back())//ÓÐÐÂ²Ù×÷
		arrAddr.back().index.push_back(Quas.size()-1);
}

void Quaternary::set(string pos, string cont) {
	if (pos == "op")
		this->op = cont;
	else if (pos == "arg1")
		this->arg1 = cont;
	else if (pos == "arg2")
		this->arg2 = cont;
	else if (pos == "result")
		this->result = cont;
	else
		this->index = atoi(cont.c_str());
}

void Quaternary::put() {
	cout << setw(8) << op << setw(8) << arg1 << setw(8) << arg2 << setw(8) << result << endl;
}

void stateFunction() {
	for (int i = 0; i < functions.size(); i++) {
		if (functions.at(i).funcName == items.back()) {
			errorNum = 2;
			cout << "ERROR::try to state a function that exists" << endl;
			return;
		}
	}
	cout << "State function::" << items.back() << endl;
	Function temp(items.back());
	Quaternary temp1;
	temp1.set("op", items.back());
	temp1.set("arg1", "_");
	temp1.set("arg2", "_");
	temp1.set("result", "_");
	Quas.push_back(temp1);
	items.pop_back();
	functions.push_back(temp);
	return;
}

void addQua(string op) {
	Quaternary temp;
	temp.set("op",op);
	if(curIndex.back()==Quas.size()-1)//num
		temp.set("arg1", factors.back());
	else
		temp.set("arg1", "T"+to_string(T-1));
	temp.set("arg2", "tofill");
	factors.pop_back();
	Quas.push_back(temp);
}

void fillQua() {
	int i;
	for (i = Quas.size() - 1;i>=0; i--) {
		if (Quas.at(i).retarg2() == "tofill") {
			if (curIndex.back() == Quas.size() - 1) {
				Quas.at(i).set("arg2", factors.back());
				factors.pop_back();
			}
			else {
				Quas.at(i).set("arg2", Quas.back().retResult());
			}
			string temp = string("T") + to_string(T++);
			Quas.at(i).set("result", temp.c_str());
			factors.push_back(temp);
			Quaternary ptr = Quas.at(i);
			for (int j = i+1; j < Quas.size(); j++) {			 
				Quas.at(j - 1) = Quas.at(j);
			}
			Quas.at(Quas.size() - 1) = ptr;
			return;
		}
	}
}

void assignArr() {
	Quaternary temp;
	temp.set("op", "[]=");
	if (curIndex.back() != Quas.size() - 1) {
		temp.set("arg1", "T" + to_string(T - 1));
	}
	else {
		temp.set("arg1", factors.back());
		factors.pop_back();
	}
	curIndex.pop_back();
	temp.set("arg2", "_");
	int res = leftIndex.back();
	temp.set("result", "T" + to_string(res) + "[T" + to_string(res-1) + "]");
	Quas.push_back(temp);
	leftIndex.pop_back();
	isLeft = true;
}

void assignVari() {
	Quaternary temp;
	temp.set("op", "=");
	if (curIndex.back() != Quas.size() - 1) {
		temp.set("arg1", "T" + to_string(T - 1));
	}
	else {
		temp.set("arg1", factors.back());
		factors.pop_back();
	}
	curIndex.pop_back();
	temp.set("arg2", "_");
	temp.set("result", factors.back());
	factors.pop_back();
	Quas.push_back(temp);
}

void retur(bool isEmpty) {
	Quaternary temp;
	temp.set("op", "ret");
	if (!isEmpty) {
		if (curIndex.back() != Quas.size() - 1) {
			temp.set("arg1", "T" + to_string(T - 1));
		}
		else {
			temp.set("arg1", factors.back());
			factors.pop_back();
		}
	}
	else
		temp.set("arg1", "_");
	temp.set("arg2", "_");
	string res;
	if (functions.size() >= 1)
		res = functions.at(functions.size()-1).funcName;
	else
		res = "programFinish";
	temp.set("result", res);
	Quas.push_back(temp);

}

void insertTempQua(string op) {
	Quaternary temp;
	temp.set("op", op);
	if (curIndex.back() != Quas.size() - 1) {
		temp.set("arg1", "T" + to_string(T - 1));
	}
	else {
		temp.set("arg1", factors.back());
		factors.pop_back();
	}
	curIndex.pop_back();
	temp.set("arg2", "tofill");
	temp.set("result", "+3");
	tempQuas.push_back(temp);

	temp.set("op", "=");
	temp.set("arg1", "0");
	temp.set("arg2", "_");
	temp.set("result", "T" + to_string(T));
	tempQuas.push_back(temp);

	temp.set("op", "j");
	temp.set("arg1", "_");
	temp.set("arg2", "_");
	temp.set("result", "+2");
	tempQuas.push_back(temp);

	temp.set("op", "=");
	temp.set("arg1", "1");
	temp.set("arg2", "_");
	temp.set("result", "T" + to_string(T));
	tempQuas.push_back(temp);

}

void fillTempQua() {
	for (int i = tempQuas.size() - 1; i >= 0; i--) {
		if (tempQuas.at(i).retarg2() == "tofill") {
			if (curIndex.back() != Quas.size() - 1) {
				tempQuas.at(i).set("arg2", "T" + to_string(T - 1));
			}
			else {
				tempQuas.at(i).set("arg2", factors.back());
				factors.pop_back();
			}
			int curT = Quas.size() - 1;
			tempQuas.at(i).set("result", to_string(curT + 5));
			tempQuas.at(i + 1).set("result", "T" + to_string(T));
			tempQuas.at(i + 2).set("result", to_string(curT + 6));
			tempQuas.at(i + 3).set("result", "T" + to_string(T++));

			for (int j = i; j < tempQuas.size(); j++)
				Quas.push_back(tempQuas.at(j));
			for (int j = tempQuas.size() - 1; j >= 0; j--) {
				tempQuas.pop_back();
				if (j == i)
					break;
			}
			break;
		}
		
	}

}

void control() {
	Quaternary temp;
	temp.set("op", "j=");
	if (curIndex.back() != Quas.size() - 1) {
		temp.set("arg1", "T" + to_string(T - 1));
	}
	else {
		temp.set("arg1", factors.back());
		factors.pop_back();
	}
	temp.set("arg2", "0");
	temp.set("result", "tofill");
	curIndex.pop_back();
	Quas.push_back(temp);
}

void endControl(bool isWhile) {
	if (isWhile) {
		Quaternary temp;
		temp.set("op", "j");
		temp.set("arg1", "_");
		temp.set("arg1", "_");
		temp.set("result", to_string(controlIndex.back()+2));

		Quas.push_back(temp);
	}
	controlIndex.pop_back();
	for (int i = Quas.size() - 1; i >= 0; i--) {
		if (Quas.at(i).retResult() == "tofill") {
			Quas.at(i).set("result", to_string(Quas.size()+1));
			break;
		}
	}
}

void callFunc() {
	Quaternary temp;
	temp.set("op", "call");
	temp.set("arg2", "_");
	temp.set("arg1", "entry(" + factors.back() + ")");
	temp.set("result", "T"+ to_string(T++));
	factors.pop_back();
	Quas.push_back(temp);
}

void sendPara() {
	Quaternary temp;
	temp.set("op", "para");
	if (curIndex.back() != Quas.size() - 1) {
		temp.set("result", "T" + to_string(T - 1));
	}
	else {
		temp.set("result", factors.back());
		factors.pop_back();
	}
	curIndex.pop_back();
	temp.set("arg1", "_");
	temp.set("arg2", "_");
	Quas.push_back(temp);
}

void tempParaList() {
	Quaternary temp;
	temp.set("op", "defpara");
	temp.set("arg1", "_");
	temp.set("arg2", "_");
	temp.set("result", items.back());
	Quas.push_back(temp);
}