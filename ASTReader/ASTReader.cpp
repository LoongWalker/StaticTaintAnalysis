#include "AST.h"
#include "callgraph.h"
#include "classTmap.h"
#include "CFGtattr.h"
//Callgraph
std::vector<callgraph*> Callgraph;
std::vector<classTmap*> ClassTmap;
//void printCallGraph(std::vector<callgraph*> Callgraph);

int main(int argc, char *argv[]) {

	FileSystemOptions opts;
	IntrusiveRefCntPtr<DiagnosticsEngine> Diags = CompilerInstance::createDiagnostics(new DiagnosticOptions());
	CompilerInstance compiler;
	//获取ASTUnit
	std::unique_ptr<ASTUnit> AU = ASTUnit::LoadFromASTFile(argv[1], compiler.getPCHContainerReader(), Diags, opts);
	ASTContext &context = AU->getASTContext();

	//扫描AST获取类定义
	ASTCXXRecordLoad loadClass;
	loadClass.HandleTranslationUnit(context);
	std::vector<CXXRecordDecl*>   cxxrds = loadClass.getClassDecl();

	if (cxxrds.size() > 0)//类的数目判断
	{
		std::vector<CXXRecordDecl*>::iterator rd_it, rd_it_end = cxxrds.end();
		for (rd_it = cxxrds.begin(); rd_it != rd_it_end; rd_it++)
		{
			//在ClassTmap中加入新发现的class
			classTmap* curClass = new classTmap();
			curClass->setCXXRecordDecl(*rd_it);
			ClassTmap.push_back(curClass);
			//添加类方法decl
			ASTCXXMethodDeclLoad loadClassMethod;
			std::vector<CXXMethodDecl *> cxxmds;
			std::vector<CXXMethodDecl *>::iterator it_cxxmds;
			//std::cout << (*rd_it)->getQualifiedNameAsString() << "\n";
			loadClassMethod.TraverseDecl(*rd_it);
			cxxmds = loadClassMethod.getCXXMethodDecl();
			if (cxxmds.size() > 0)
			{
				for (it_cxxmds = cxxmds.begin(); it_cxxmds != cxxmds.end(); it_cxxmds++)
				{
					curClass->addMethod(*it_cxxmds);
					callgraph* tempClassMethodNode = new callgraph(*it_cxxmds);
					Callgraph.insert(Callgraph.begin(), tempClassMethodNode);
					//std::cout << (*it_cxxmds)->getQualifiedNameAsString() << "\n";
				}
			}
			//添加类变量decl(类型为fieldDecl*)
			//FieldDecl* fd;

			ASTFieldDeclLoad loadClassVar;
			std::vector<FieldDecl*> fds;
			std::vector<FieldDecl*>::iterator it_fds;
			loadClassVar.TraverseDecl(*rd_it);
			fds = loadClassVar.getFieldDecl();
			if (fds.size() > 0)
			{
				for (it_fds = fds.begin(); it_fds != fds.end(); it_fds++)
				{
					curClass->addVar(*it_fds);
					//std::cout << (*it_fds)->getQualifiedNameAsString() << "\n";
				}
			}
		}
	}

	//扫描AST获取vector::<functionDecl>
	ASTFunctionLoad load;
	load.HandleTranslationUnit(context);
	std::vector<FunctionDecl*>  func = load.getFunctions();

	std::vector<FunctionDecl*>::iterator it_func_decl;

	std::vector<CallExpr*>  callExpr;
	std::vector<CallExpr*>::iterator it_call_expr;
	std::vector<VarDecl*> varDec;
	std::vector<VarDecl*>::iterator itvarDec;
	std::vector<ParmVarDecl*> parmDec;
	std::vector<ParmVarDecl*>::iterator itParmDec;

	//add cur->callee
	if (func.size() > 0)
	{
		//为functionDecl添加callgraph
		callgraph* tempCallNode;
		for (it_func_decl = func.begin(); it_func_decl != func.end(); it_func_decl++)
		{
			tempCallNode = new callgraph(*it_func_decl);
			Callgraph.insert(Callgraph.begin(), tempCallNode);
		}
	}

	//为所有方法添加函数调用关系
	std::vector<callgraph*>::iterator it_callgraph;
	if (Callgraph.size() > 0)
	{
		FunctionDecl* cur;
		for (it_callgraph = Callgraph.begin(); it_callgraph != Callgraph.end(); it_callgraph++)
		{
			cur = (*it_callgraph)->getCur();
			ASTCallExprLoad load2;
			load2.TraverseStmt(cur->getBody());
			callExpr = load2.getCallExprs();
			if (callExpr.size() > 0)
			{
				//添加调用的函数callee
				for (it_call_expr = callExpr.begin(); it_call_expr != callExpr.end(); it_call_expr++)
				{
					(*it_callgraph)->addCallee((*it_call_expr)->getDirectCallee());
					//	std::cout << (*it)->getQualifiedNameAsString() << " -> " << (*it_call_expr)->getDirectCallee()->getQualifiedNameAsString() << std::endl;
				}
				//添加调用本函数的函数caller
				for (it_call_expr = callExpr.begin(); it_call_expr != callExpr.end(); it_call_expr++)
				{
					callgraph* tempCallgraph = findById(Callgraph, cur->getQualifiedNameAsString());
					tempCallgraph->addCallee((*it_call_expr)->getDirectCallee());
					//	std::cout << (*it)->getQualifiedNameAsString() << " -> " << (*it_call_expr)->getDirectCallee()->getQualifiedNameAsString() << std::endl;
				}
			}
			//把参数加入Tmap
			int paramNum = cur->getNumParams();
			//VarDecl* parmTemp;
			if (paramNum > 0)
			{
				for (int i = 0; i < paramNum; i++)
				{
					(*it_callgraph)->addParam(cur->getParamDecl(i));
					(*it_callgraph)->getCTmap().setAttr(cur->getParamDecl(i), RELATED, 1 << i);
				}
			}

			//把中间变量加入Tmap
			ASTVarDeclLoad loadVar;
			loadVar.TraverseStmt(cur->getBody());
			std::vector<VarDecl*> varList = loadVar.getVariables();
			for (std::vector<VarDecl*>::iterator varIt = varList.begin(); varIt != varList.end(); varIt++)
			{
				//std::cout << (*varIt)->getQualifiedNameAsString() << "\n";
				(*it_callgraph)->addVar(*varIt);
			}
		}

		/*
		//添加调用本函数的其他函数caller
		for (it_func_decl = func.begin(); it_func_decl != func.end(); it_func_decl++)
		{
		ASTCallExprLoad load2;
		load2.TraverseStmt((*it_func_decl)->getBody());
		callExpr = load2.getCallExprs();
		for (it_call_expr = callExpr.begin(); it_call_expr != callExpr.end(); it_call_expr++)
		{
		for (it_callgraph = Callgraph.begin(); it_callgraph != Callgraph.end(); it_callgraph++)
		{
		if ((*it_callgraph)->getCur() == (*it_call_expr)->getDirectCallee())
		(*it_callgraph)->addCaller(*it_func_decl);
		}
		}
		}
		*/
	}

	std::vector<FunctionDecl*> ringVector;
	getRing(Callgraph, 0, ringVector);

	resetIfCheck(Callgraph);
	ifcheck(Callgraph, *Callgraph.begin());

	printCallGraph(Callgraph);

	printClassTmap(ClassTmap);

	std::vector<callgraph*>::iterator it3;
	it3 = Callgraph.begin();
	for (; it3 != Callgraph.end(); it3++)
	{
		checkCFG((*it3)->get_cfg(), (*it3)->getCTmap(), *it3);
	}

	return 0;
}
