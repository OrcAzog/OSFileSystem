#pragma once
#include"structs.h"
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
//extern int umask;
void initBlockStack();
void realseEmptyBlock(int i);
bool checkUser(Inode * inode, int tar);
void buildNewUser(string group, string u_name, string password);
dentry* findDentry(dentry *d, vector<string>strs, bool flag);
template<class T>
int buildNewFile(T &c, int size, int val = -1);
void LoadSuperBlock();
void LoadInodes();
void LoadBlockMap();
int findEmptyNode();
int findEmptyBlock();
template <typename T>
void write(fstream &f, T &t);
template <typename T>
void read(fstream &f, T &t);
int  findGroup(char * c);
int getBlockInnode(int k, Inode * node);
vector<int> getEmptyBlockVec(int n);
void writeIntoNode(char *c, Inode * node, int size, bool reWrite);
int readFromNode(Inode * node, int size = -1, int start = 0);
void writeIntoNodeWithCheck(char *c, Inode * node, int size, bool reWrite);
int readFromNodeWithCheck(Inode * node, int size , int start);

void save();
dentry* buildDentry(FCB fcb, bool flag, dentry *d = NULL);
void Load();
template<class T>
int buildNewFile(T &c, int size, int val) {
	if (val <= 0)
		val = findEmptyNode();
	Inode *node = &inodes[val];
	//cout << &c;
	//cout << buff;
	writeIntoNode((char*)&c, node, size, false);
	return val;
}