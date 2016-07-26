#include <iostream>
#include <vector>

#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/FileSystemOptions.h"
#include "clang/AST/RecursiveASTVisitor.h"

using namespace clang;
using namespace llvm;
//using namespace std;

class ASTFunctionLoad : public ASTConsumer, public RecursiveASTVisitor < ASTFunctionLoad > {	//get functions

public:
	void HandleTranslationUnit(ASTContext &Context) override {
		TranslationUnitDecl *D = Context.getTranslationUnitDecl();
		TraverseDecl(D);
	}

	bool VisitFunctionDecl(FunctionDecl *FD) {
		if (FD && FD->isThisDeclarationADefinition()) {
			// Add C non-inline function 
			if (!FD->isInlined()){
				functions.push_back(FD);
			}
		}
		return true;
	}

	const std::vector<FunctionDecl *> &getFunctions() const{
		return functions;
	}

private:
	std::vector<FunctionDecl *> functions;
};

class ASTCalledFunctionLoad : public RecursiveASTVisitor<ASTCalledFunctionLoad> {	//get call functions

public:
	bool VisitCallExpr(CallExpr *E) {
		if (FunctionDecl *FD = E->getDirectCallee()) {
			functions.insert(FD);
		}
		return true;
	}

	const std::vector<FunctionDecl *> getFunctions() {
		return std::vector<FunctionDecl *>(functions.begin(), functions.end());
	}

private:
	std::set<FunctionDecl *> functions;
};

class ASTCallExprLoad : public RecursiveASTVisitor<ASTCallExprLoad> {

public:
	bool VisitCallExpr(CallExpr *E) {
		call_exprs.push_back(E);
		return true;
	}

	const std::vector<CallExpr *> getCallExprs() {
		return call_exprs;
	}

private:
	std::vector<CallExpr *> call_exprs;
};

class ASTVariableLoad : public RecursiveASTVisitor<ASTVariableLoad> {

public:
	bool VisitDeclStmt(DeclStmt *S) {
		for (auto D : S->decls()) {
			if (VarDecl *VD = dyn_cast<VarDecl>(D)) {
				variables.push_back(VD);
			}
		}
		return true;
	}

	const std::vector<VarDecl *> getVariables() {
		return variables;
	}

private:
	std::vector<VarDecl *> variables;
};


class callgraph{
public:
	callgraph(FunctionDecl* f1)
	{
		cur = f1;
		callerNum = 0;
		calleeNum = 0;
		ifCheck = 0;
	}
	callgraph(FunctionDecl* f1, FunctionDecl* f2)
	{
		cur = f1;
		callee[0] = f2;
		callerNum = 0;
		calleeNum = 1;
		ifCheck = 0;
	}
	FunctionDecl* getCaller(int i);
	FunctionDecl* getCallee(int i);
	FunctionDecl* getCur();
	int getCallerNum();
	int getCalleeNum();
	void addCaller(FunctionDecl* otherFD);
	void addCallee(FunctionDecl* otherFD);
	void delCallee(FunctionDecl* otherFD);
	int ifCheck;
private:
	FunctionDecl* caller[10];
	FunctionDecl* cur;
	FunctionDecl* callee[10];
	int callerNum, calleeNum;
};

//callgraph* findById(std::vector<callgraph*> Callgraph, std::string id);

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

FunctionDecl* callgraph::getCaller(int i)
{
	if (0 <= i&& i < 10)
		return caller[i];
	else
		return NULL;
}

FunctionDecl* callgraph::getCallee(int i)
{
	if (0 <= i&&i < 10)
		return callee[i];
	else
		return NULL;
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
	caller[callerNum] = otherFD;
	callerNum++;
}

void callgraph::addCallee(FunctionDecl* otherFD){
	callee[calleeNum] = otherFD;
	calleeNum++;
}


void callgraph::delCallee(FunctionDecl* otherFD)
{
	int j;
	for (int i = 0; i < calleeNum&&i < 10; i++)
		if (callee[i] == otherFD)
		{
			j = i;
			break;
		}
	for (int i = j; i < calleeNum - 1 && i < 9; i++)
	{
		callee[i] = callee[i + 1];
	}
	callee[callerNum] = NULL;
	calleeNum--;
}

void ifcheck(std::vector<callgraph*> cg, callgraph* t)
{
	t->ifCheck = -1;
	for (int i = 0; i < t->getCalleeNum(); i++)
	{
		FunctionDecl* tempt = t->getCallee(i);
		callgraph* tempc = findById(cg, tempt->getQualifiedNameAsString());
		if (tempc != NULL)
		{
			if (tempc->ifCheck == -1)
			{
				t->delCallee(tempt);
				i--;
			}
			else
				ifcheck(cg, tempc);
		}
	}
	t->ifCheck = 1;
}
