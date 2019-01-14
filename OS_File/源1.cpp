
#include "applicationLayer.h"

extern fstream file;
extern string cur;
extern SuperBlock  superblock;
extern Block blocks[BLOCKSUM];//块区表	
extern Inode inodes[INODESUM];//节点区表
extern Usr *curUser;
extern bool blocksTable[BLOCKSUM];
extern UsrTable usrTable;
extern dentry rootDentry;
extern dentry* currentDentry;
extern char buff[1024];
extern int umask;

void Load() {
	
	LoadSuperBlock();
	LoadInodes();
	dir baseFile;
	cur = "/";


	readFromNode(&inodes[24], sizeof(blocksTable));//读取块表
	memcpy((char*)&blocksTable, buff, sizeof(blocksTable));
	readFromNode(&inodes[0], sizeof(baseFile));//读取根目录
	memcpy((char*)&baseFile, buff, sizeof(baseFile));
	rootDentry = *buildDentry(baseFile.d_this, true);
	currentDentry = &rootDentry;
	readFromNode(&inodes[54], sizeof(usrTable));//读取用户目录
	memcpy((char*)&usrTable, buff, sizeof(usrTable));
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);
	curUser = findUser("vistor");
}





void entryPoint() {
	Load();
	string s;
	su("vistor", true);
	while (true) {
		
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_GREEN);

		cout << curUser->u_name;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_BLUE |
			FOREGROUND_RED);
		cout << " " << currentDentry->current->d_this.f_name;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);
		cout << "  #  ";
		cin >> s;
		if (s == "ls")ls();
		else if (s == "mkdir") {

			cin >> s; mkdir(s);

		}
		else if (s == "cd") {

			cin >> s; cd(s);
		}
		else if (s == "mkfile") {

			cin >> s; mkfile(s);
		}
		else if (s == "pwd") {

			pwd();
		}
		else if (s == "su") {
			cin >> s;
			su(s,false);
		}
		else if (s == "cat") {
			cin >> s;
			cat(s);
		}
		else if (s == "chmod") {
			cin >> s;
			chmod(s);
		}
		else if (s == "umask") {
			cin >> s;
			cat(s);
		}
		else {

		}
		cout << endl;
	}
}

int getasd() {
	emptyBlockTable *b= &superblock.emptyblockTable;
	//memcpy(b, &blocks[superblock.emptyblockTable.vals[0]], sizeof(emptyBlockTable));
	if (b->vals[0] == 0&&b->sum<=1)
	{
		cout << "分配完了";
		return -1;
	}
	if (b->sum == 1) {
		int temp = b->vals[0];
		memcpy(b, &blocks[b->vals[0]], sizeof(emptyBlockTable));
		return temp;
	}
	
	return b->vals[--b->sum];
}
void main() {
	initBlockStack();
	for (int i = 0; i < 1000; i++) {
		int j = getasd();
		cout << j<<endl;
	}
}

