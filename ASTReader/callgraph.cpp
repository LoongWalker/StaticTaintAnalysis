#include "callgraph.h"
//callgraph构造函数
callgraph::callgraph(FunctionDecl* f1)
{
	cur = f1;
	callerNum = 0;
	calleeNum = 0;
	ifCheck = 0;
	type = common;
	classDecl = NULL;
	map = new CTmap();
	paramNum = 0;
	varNum = 0;
	returnVar = new Tainted_Attr();
}

//calllgraph构造函数（带有一个callee）
callgraph::callgraph(FunctionDecl* f1, FunctionDecl* f2)
{
	cur = f1;
	callee.push_back(f2);
	callerNum = 0;
	calleeNum = 1;
	ifCheck = 0;
	type = common;
	classDecl = NULL;
	map = new CTmap();
	paramNum = 0;
	varNum = 0;
	returnVar = new Tainted_Attr();
}

std::vector<FunctionDecl*>& callgraph::getCaller()
{
	return caller;
}

std::vector<FunctionDecl*>& callgraph::getCallee()
{
	return callee;
}

FunctionDecl* callgraph::getCur()
{
	return cur;
}

int callgraph::getCallerNum()
{
	return callerNum;
}

int callgraph::getCalleeNum()
{
	return calleeNum;
}

void callgraph::addCaller(FunctionDecl* otherFD){
	caller.push_back(otherFD);
	callerNum++;
}

void callgraph::addCallee(FunctionDecl* otherFD){
	callee.push_back(otherFD);
	calleeNum++;
}


void callgraph::delCallee(FunctionDecl* otherFD)
{
	std::vector<FunctionDecl*>::iterator it_caller;
	std::vector<FunctionDecl*>::iterator it_callee;
	for (it_callee = callee.begin(); it_callee != callee.end(); it_callee++)
		if ((*it_callee) == otherFD)
		{
			callee.erase(it_callee);
			break;
		}
}

//函数名查找callgraph实例
callgraph* findById(std::vector<callgraph*> Callgraph, std::string id)
{
	std::vector<callgraph*>::iterator it;
	for (it = Callgraph.begin(); it != Callgraph.end(); it++)
	{
		if (id == (*it)->getCur()->getQualifiedNameAsString())
			return *it;
	}
	return NULL;
}

//查找callgraph中的环，对其进行除环操作
void ifcheck(std::vector<callgraph*> cg, callgraph* t)
{
	t->ifCheck = -1;
	std::vector<FunctionDecl*>::iterator it_callee;
	std::vector<FunctionDecl*> callee = t->getCallee();
	for (it_callee = callee.begin(); it_callee != callee.end(); it_callee++)
	{
		FunctionDecl* tempt = *it_callee;
		callgraph* tempc = findById(cg, tempt->getQualifiedNameAsString());
		if (tempc != NULL)
		{
			if (tempc->ifCheck == -1)
			{
				t->delCallee(tempt);
				it_callee--;//=================
			}
			else
				ifcheck(cg, tempc);
		}
	}
	t->ifCheck = 1;
}

//重置ifcheck
void resetIfCheck(std::vector<callgraph*>Callgraph)
{
	std::vector<callgraph*>::iterator it;
	for (it = Callgraph.begin(); it < Callgraph.end(); it++)
		(*it)->ifCheck = 0;
}

//打印callgraph中存在的环的信息
void getRing(std::vector<callgraph*>&Callgraph, int n, std::vector<FunctionDecl*>& ringVector)
{
	std::stack<FunctionDecl* > ring;
	std::vector<callgraph*>::iterator it, it4;
	std::vector<FunctionDecl*>::iterator it2, it3;
	std::vector<FunctionDecl*>::iterator it_callee;

	//FunctionDecl* temp;
	it = Callgraph.begin() + n;
	if ((*it)->ifCheck == 0)//new node
	{
		std::vector<FunctionDecl*> callee = (*it)->getCallee();
		ringVector.push_back((*it)->getCur());
		(*it)->ifCheck = -1;
		for (it_callee = callee.begin(); it_callee != callee.end(); it_callee++)
		{
			for (it4 = Callgraph.begin(); it4 < Callgraph.end(); it4++)
				if ((*it4)->getCur() == *it_callee)
				{
					getRing(Callgraph, it4 - Callgraph.begin(), ringVector);
					break;
				}
		}
		(*it)->ifCheck = 1;
		ringVector.pop_back();
	}
	else if ((*it)->ifCheck == -1)
	{
		for (it2 = ringVector.begin(); it2 < ringVector.end(); it2++)
		{
			if ((*it2) == (*it)->getCur())
				break;
		}
		//print
		for (it3 = it2; it3 < ringVector.end(); it3++)
		{
			std::cout << (*it3)->getQualifiedNameAsString() << " ";
		}
		std::cout << std::endl;
	}
	else//ifCheck==1
	{
		return;
	}
}

void printCallGraph(std::vector<callgraph*> Callgraph)
{
	std::vector<callgraph*>::iterator it3;

	std::vector<FunctionDecl*>::iterator it_call;
	std::vector<FunctionDecl*> caller;
	std::vector<FunctionDecl*> callee;
	for (it3 = Callgraph.begin(); it3 != Callgraph.end(); it3++)
	{
		caller = (*it3)->getCaller();
		callee = (*it3)->getCallee();
		std::cout << (*it3)->getCur()->getQualifiedNameAsString() << "\n";
		int paramNum = (*it3)->getCur()->getNumParams();
		int varNum = (*it3)->getVarNum();
		CTmap map = (*it3)->getCTmap();

		std::cout << "\tParamNum: " << paramNum << "\n";
		if (paramNum > 0)
		{
			for (unsigned i = 0; i < (*it3)->getCur()->getNumParams(); i++)
			{
				VarDecl* tt = map.get_VarDecl(i);
				std::cout << "\t\t" << tt->getQualifiedNameAsString() << "    ";
				map.getmap(tt)->output();
				std::cout << "\n";
			}
		}

		std::cout << "\tVarNum: " << varNum << "\n";
		if (varNum > 0)
		{
			for (int i = 0; i < varNum; i++)
			{
				VarDecl* tt = map.get_VarDecl(i + paramNum);
				std::cout << "\t\t" << tt->getQualifiedNameAsString() << "    ";
				map.getmap(tt)->output();
				std::cout << "\n";
			}
		}

		int j = (*it3)->getCallerNum();
		std::cout << "\tcaller:" << j << "\n";
		for (it_call = caller.begin(); it_call != caller.end(); it_call++)
			std::cout << "\t\t" << (*it_call)->getQualifiedNameAsString() << "\n";
		j = (*it3)->getCalleeNum();
		std::cout << "\tcallee:" << j << "\n";
		for (it_call = callee.begin(); it_call != callee.end(); it_call++)
			std::cout << "\t\t" << (*it_call)->getQualifiedNameAsString() << "\n";
	}
}

//改变方法的类型（普通，类方法）
void callgraph::changeMethodType()
{
	if (type == common)
		type = inclass;
	else
		type = common;
}

//设置方法的实例指针
void callgraph::setRoot(VarDecl* r)
{
	root = r;
}

//获取实例指针
VarDecl* callgraph::getRoot()
{
	return root;
}

//获取方法类型
methodType callgraph::getMethodType()
{
	return type;
}

//设置类定义的指针
void callgraph::setClass(CXXRecordDecl* rd)
{
	classDecl = rd;
}
CXXRecordDecl* callgraph::getClass()
{
	return classDecl;
}

//获取当前函数语句块信息指针
std::unique_ptr<CFG>& callgraph::get_cfg()
{
	cfg = CFG::buildCFG(cur, cur->getBody(), &cur->getASTContext(), CFG::BuildOptions());
	return cfg;
}

//打印cfg信息，使用dump()
void callgraph::print_cfg()
{
	(*cfg).dump(LangOptions(), true);
}

//获取当前函数Tmap指针
CTmap& callgraph::getCTmap()
{
	return *map;
}

//添加参数
void callgraph::addParam(VarDecl* vd)
{
	map->insert(vd);
	paramNum++;
}

//添加变量
void callgraph::addVar(VarDecl* vd)
{
	map->insert(vd);
	varNum++;
}

int callgraph::getParamNum()
{
	return paramNum;
}

int callgraph::getVarNum()
{
	return varNum;
}
