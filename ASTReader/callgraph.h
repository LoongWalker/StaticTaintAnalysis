#ifndef CALLGRAPH_H
#define CALLGRAPH_H


#include "AST.h"

typedef enum
{
	common, inclass
}methodType;

//函数调用关系图
class callgraph{
public:
	callgraph(FunctionDecl* f1);
	callgraph(FunctionDecl* f1, FunctionDecl* f2);
	std::vector<FunctionDecl*>& getCaller();		//调用cur
	std::vector<FunctionDecl*>& getCallee();		//被cur调用
	FunctionDecl* getCur();
	int getCallerNum();
	int getCalleeNum();
	void addCaller(FunctionDecl* otherFD);
	void addCallee(FunctionDecl* otherFD);
	void delCallee(FunctionDecl* otherFD);
	void changeMethodType();
	void setRoot(VarDecl* r);
	void setClass(CXXRecordDecl* rd);
	CXXRecordDecl* getClass();
	VarDecl* getRoot();
	methodType getMethodType();
	int ifCheck;

public:
	std::unique_ptr<CFG>& get_cfg();
	void print_cfg();
	CTmap& getCTmap();
	void addParam(VarDecl* vd);
	void addVar(VarDecl* vd);
	int getParamNum();
	int getVarNum();

private:
	//方法的类型
	methodType type;
	//类方法所属的实例
	VarDecl* root;
	CXXRecordDecl* classDecl;
	std::vector<FunctionDecl*> caller;
	FunctionDecl* cur;
	std::vector<FunctionDecl*> callee;
	std::unique_ptr<CFG> cfg;
	int callerNum, calleeNum;
	CTmap* map;
	int paramNum;
	int varNum;
	Tainted_Attr* returnVar;
};

callgraph* findById(std::vector<callgraph*> Callgraph, std::string id);
void ifcheck(std::vector<callgraph*> cg, callgraph* t);
void resetIfCheck(std::vector<callgraph*>Callgraph);
void getRing(std::vector<callgraph*>& Callgraph, int n, std::vector<FunctionDecl*>& ringVector);
void printCallGraph(std::vector<callgraph*> Callgraph);
#endif
