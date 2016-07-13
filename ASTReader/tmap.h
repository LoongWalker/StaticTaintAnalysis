#ifndef TMAP_H
#define TMAP_H

#include <iostream>
#include <map>
#include <vector>

#include "clang/Frontend/ASTUnit.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/FileSystemOptions.h"
#include "clang/AST/RecursiveASTVisitor.h"

using namespace std;
using namespace clang;
using namespace llvm;

//污染情况
typedef enum{
	TAINTED,
	UNTAINTED,
	RELATED
}e_tattr;

//变量的污染属性
class Tainted_Attr
{
public:
	friend class CTmap;
	//构造函数
	Tainted_Attr(){
		attr = UNTAINTED;
		relation = 0;
	}
	//拷贝构造函数
	Tainted_Attr(Tainted_Attr& b)
	{
		attr = b.attr;
		relation = b.relation;
	}
	//信息输出函数 调试用
	void output()
	{
		if (attr == TAINTED)
			std::cout << "TAINTED ";
		else if (attr == UNTAINTED)
			std::cout << "UN ";
		else
			std::cout << "RE ";
		std::cout << relation;
	}
	//信息设置函数
	void attr_set(e_tattr a, unsigned long long r)
	{
		attr = a;
		relation |= r;
	}
	//将两个污染属性取并
	void AndAttr(Tainted_Attr &b)
	{
		if (attr == TAINTED)
			return;
		if (b.attr == TAINTED)
		{
			attr = TAINTED;
			return;
		}
		if (attr == UNTAINTED)
		{
			attr = b.attr;
			relation = b.relation;
			return;
		}
		if (b.attr == UNTAINTED)
			return;
		attr = b.attr;
		relation |= b.relation;
	}
public:
	//污染属性
	e_tattr attr;
	//污染与哪些变量相关
	unsigned long long relation;
};

//封装了C++ map模板的污染表类 
class CTmap
{
	friend class CFGInOut;
public:
	//构造函数
	CTmap(){}
	//拷贝构造函数
	CTmap(CTmap& b)
	{
		Tainted_Attr *t = NULL, *newattr;
		VarDecl *pdec = NULL;
		map<VarDecl *, Tainted_Attr *>::iterator it = b.tmap.begin(), it_end = b.tmap.end();

		while (it != it_end)
		{
			pdec = (*it).first;
			t = (*it).second;
			newattr = new Tainted_Attr;

			newattr->attr = t->attr;
			newattr->relation = t->relation;

			tmap[pdec] = newattr;
			it++;
		}
	}
	//析构函数
	~CTmap()
	{
		Tainted_Attr *t;
		map<VarDecl *, Tainted_Attr *>::iterator iter = tmap.begin(), iter_end = tmap.end();

		while (iter != iter_end)
		{
			t = (*iter).second;
			delete t;	//释放临时变量的空间
			iter++;
		}
		tmap.clear();	//清空所有元素
	}

	void output()
	{
		map<VarDecl *, Tainted_Attr *>::iterator iter = tmap.begin(), iter_end = tmap.end();
		while (iter != iter_end)
		{
			std::cout << iter->first->getQualifiedNameAsString() << " ";
			iter->second->output();
			cout << endl;
			iter++;
		}
	}

	void CopyMap(CTmap& b)
	{
		tmap.clear();
		Tainted_Attr *t = NULL, *newattr;
		VarDecl *pdec = NULL;
		map<VarDecl *, Tainted_Attr *>::iterator it = b.tmap.begin(), it_end = b.tmap.end();

		while (it != it_end)
		{
			pdec = (*it).first;
			t = (*it).second;
			newattr = new Tainted_Attr;

			newattr->attr = t->attr;
			newattr->relation = t->relation;

			tmap[pdec] = newattr;
			it++;
		}
	}

	//若p不在表中，插入一个变量定义节点，并创建一个污染属性变量
	void insert(VarDecl *p)
	{
		Tainted_Attr *t = new Tainted_Attr();

		int count;
		count = tmap.count(p);
		if (count == 0)
			tmap[p] = t;
		else
			delete t;
	}

	//从map中删除p
	void del(VarDecl *p)
	{
		Tainted_Attr *t = tmap[p];
		delete t;
		tmap.erase(p);
	}

	//取得变量定义节点p对应的污染属性
	Tainted_Attr *getmap(VarDecl *p)
	{
		int count;
		count = tmap.count(p);
		if (count == 0)
			return NULL;
		else
			return tmap[p];
	}

	//对某个变量的污染属性值进行设置
	void setAttr(VarDecl *p, e_tattr a, unsigned long long r)
	{
		int count;
		count = tmap.count(p);
		if (count == 0)
		{
			cout << "No such variable in the function" << endl;
			return;
		}
		else
		{
			tmap[p]->attr_set(a, r);
		}
	}

	//将两个map中的污染属性合并
	void AndMap(CTmap &b)
	{
		VarDecl *p;
		map<VarDecl *, Tainted_Attr *>::iterator iter = tmap.begin(), iter_end = tmap.end();
		while (iter != iter_end)
		{
			p = (*iter).first;
			if (b.getmap(p) != NULL)
				(*iter).second->AndAttr(*b.getmap(p));
			iter++;
		}
	}

	//清空map中的元素
	void clear()
	{
		tmap.clear();
	}

	//获取第n个变量定义
	VarDecl *get_VarDecl(int n)
	{
		int i = 0;
		map<VarDecl *, Tainted_Attr *>::iterator iter = tmap.begin(), iter_end = tmap.end();
		for (; iter != iter_end; iter++)
		{
			if (i == n)
				return iter->first;
			i++;
		}
		return NULL;
	}
private:
	map<VarDecl *, Tainted_Attr *> tmap;
};

#endif
