#include "AST.h"
class classTmap
{
public:
	classTmap();
	void setCXXRecordDecl(CXXRecordDecl* cxxrd);
	void addMethod(CXXMethodDecl* md);
	void addVar(FieldDecl* fd);
	const std::vector<CXXMethodDecl*>& get_cxxmds();
	const std::vector<FieldDecl*>& get_fds();
	CXXRecordDecl* get_cxxrd()const;
	int getMethodNum();
	int getVarNum();
private:
	CXXRecordDecl* rd;
	std::vector<CXXMethodDecl*> cxxmds;
	std::vector<FieldDecl*> fds;
	int methodNum;
	int varNum;
	//int publicVarNum;
	//int privateVarNum;
	//int protectedVarNum;
};

void printClassTmap(std::vector<classTmap*> CT);
