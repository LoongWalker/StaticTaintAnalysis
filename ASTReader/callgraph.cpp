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
	return_tattr = new Tainted_Attr();
	if_check_cfg = false;
	return_relation=0;
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
	return_tattr = new Tainted_Attr();
	if_check_cfg = false;
	return_relation=0;
}

callgraph::~callgraph()
{
	block_io_map.clear();
	TCI_list.clear();
	TCI_list_call.clear();
	caller.clear();
	callee.clear();
	delete(cfg.get());
	map->~CTmap();
}

std::vector<FunctionDecl*>& callgraph::getCaller()
{
	return caller;
}

std::vector<FunctionDecl*>& callgraph::getCallee()
{
	return callee;
}

//获得callgraph的所属函数的functiondecl
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

//根据函数名查找callgraph
callgraph* findById(std::vector<callgraph*> Callgraph, std::string id)
{
	std::vector<callgraph*>::iterator it;
	for (it = Callgraph.begin(); it != Callgraph.end(); it++)
	{
		if ((*it)->getCur()->getQualifiedNameAsString().compare(id)==0)
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

//打印callgraph相关信息
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
				const VarDecl* tt = map.get_VarDecl(i);
				std::cout << "\t\t" << tt->getQualifiedNameAsString() << "    ";
				map.getAttr(tt)->output();
				std::cout << "\n";
			}
		}

		std::cout << "\tVarNum: " << varNum << "\n";
		if (varNum > 0)
		{
			for (int i = 0; i < varNum; i++)
			{
				const VarDecl* tt = map.get_VarDecl(i + paramNum);
				std::cout << "\t\t" << tt->getQualifiedNameAsString() << "    ";
				map.getAttr(tt)->output();
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
	map_param.push_back(vd);
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

//检测某函数是否已经是本函数所调用的函数
bool callgraph::is_caller(FunctionDecl* fd)
{
	int size = caller.size();
	if (size == 0)
		return false;
	for (int i = 0; i < size; i++)
	{
		if (caller[i] == fd)
			return true;
	}
	return false;
}

//检测某函数是否已经是调用本函数的函数
bool callgraph::is_callee(FunctionDecl* fd)
{
	int size = callee.size();
	if (size == 0)
		return false;
	for (int i = 0; i < size; i++)
	{
		if (callee[i] == fd)
			return true;
	}
	return false;
}

//检测某函数是否已经生成了他的callgraph
bool if_find_function(vector<callgraph*> Callgraph, FunctionDecl* fd)
{
	if (Callgraph.size() == 0)
		return false;
	vector<callgraph*>::iterator it=Callgraph.begin(),it_end=Callgraph.end();
	for (; it != it_end; it++)
	{
		if ((*it)->getCur() == fd)
			return true;
	}
	return false;
}


Tainted_Attr* callgraph::get_return()
{
	return return_tattr;
}

void callgraph::set_return(Tainted_Attr* temp)
{
	return_tattr->copy(temp);
}

//将if_check_cfg设为已经检查true
void callgraph::set_if_check_cfg()
{
	if_check_cfg = true;
}

bool callgraph::get_if_check_cfg()
{
	return if_check_cfg;
}

unsigned callgraph::get_return_relation()
{
	return return_relation;
}

//设置返回值与某一位相关
bool callgraph::set_return_relation(int i)
{
	if (i > 32)
	{
		cout << "参数数目不超过32" << endl;
		return false;
	}
	return_relation += 1 << i;
	return true;
}

//获取某个参数变量的参数编号
int callgraph::get_param_no(const VarDecl* vd)
{
	int i = 0;
	while (1)
	{
		if (map_param[i] == vd)
			return i;
		i++;
		if (i > paramNum)
			return -1;
	}
}

//设置该函数所在ast文件的astcontext
void callgraph::set_ASTContext(ASTContext* astcontext_temp)
{
	astcontext = astcontext_temp;
}

//获取该函数所在文件的sourceManager
SourceManager& callgraph::getSourceManager()
{
	return astcontext->getSourceManager();
}

//获取该函数所在文件的ASTContext
ASTContext* callgraph::getASTContext()
{
	return astcontext;
}
