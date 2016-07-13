#ifndef CFGTATTR_H
#define CFGTATTR_H

#include "tmap.h"
#include "AST.h"
#include "callgraph.h"
#include "clang\Analysis\CFG.h"

using namespace std;
using namespace clang;
using namespace llvm;

//CFG的输入输出类
class CFGInOut{
private:
	//CFG的输入输出
	CTmap IN, OUT;
public:
	//拷贝构造函数
	CFGInOut()
	{
	}
	CFGInOut(CTmap& b)
	{
		IN.CopyMap(b);
		OUT.CopyMap(b);
	}
	CTmap* GetIN()
	{
		return &IN;
	}
	CTmap* GetOUT()
	{
		return &OUT;
	}
	void setIO(CTmap &tm)
	{
		IN.CopyMap(tm);
		OUT.CopyMap(tm);
		cout << "in" << endl;
		IN.output();
		cout << "out" << endl;
		OUT.output();
	}
};

void checkCFG(std::unique_ptr<CFG> &cfg, CTmap &tm, callgraph *cg);
void build_block_io_table(map<clang::CFGBlock *, CFGInOut> &block_io_map, clang::CFGBlock *CFGexit, clang::CFGBlock *block, CTmap &tm);
void printBlockMsg(map<clang::CFGBlock *, CFGInOut> &block_io_map, clang::CFGBlock *block);
void printiotable(map<clang::CFGBlock *, CFGInOut> &block_io_map);
#endif
