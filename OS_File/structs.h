using namespace std;
#define _CRT_SECURE_NO_WARNINGS
#include<bits/stdc++.h>
#include"utils.h"
#include <windows.h>

#define FILE_CLASS "D:\\source"
#define INODE_FILE "D:\\file"
#define BLOCKSUM 1000
#define INODESUM 300
#define BlockIndexMap_MaxSum 
#define Block_Size 1024
#define Inode_Max_File 1024 * 140
#define root_index_node 0
#define file_System_size 1024*100
#define bulid

struct emptyBlockTable {
	int sum=1;
	int vals[100] = {};
	
};
struct SuperBlock {
public:
	int Sblock_sum = BLOCKSUM;
	int Sblock_bitmap_start = 24;//块位图inode点
	int Sinode_sum = INODESUM;
	int Sinode_bitmap_start = 31;//节点位图inode
	int Sinode_bitmap_size;
	int dir_entry_size = Block_Size;
	char c[592];//todo
	emptyBlockTable emptyblockTable;
	int umask;
};
struct Block {
	char c[Block_Size] = {};
};
enum file_types {
	FT_UNKNOWN,
	FT_REGULAR,
	FT_DIRECTORY,
	FT_LINK

};
struct FCB {
public:
	char f_name[18];
	int inode;
	file_types f_type;

};
struct dir {

	FCB d_this;
	FCB children[20];
	FCB *father = NULL;
	int d_count = 0;

};
struct dentry {
public:
	static dentry rootDentry;
	string str;
	dir*current;
	dentry*father = NULL;
	vector<dentry*>children;
	//cd ./root
	//cd root 
	//cd ../root

};
struct Inode {
	int i_auth;
	bool i_isDir = false;//感觉没用好吧
	int64_t LastChange;//最后修改时间
	char group[10];
	char user[10];
	int i_link = 0;
	int i_sector[6] = { -1,-1,-1,-1,-1,-1 };//四个直接块号，一个一次间址，一个两次间址 512/4+12=140
	bool isEmpty = true;
	int i_curSize = 0;
	int i_key;//区号
	int i_curByteSum;//当前总字符段
	bool i_writeDeny = false;//同步读写锁
	dentry* i_dentry;
	int i_couont;
	char c[36] = {};//占位符todo
};


struct Usr {
	char u_name[10];
	int u_group;
	char password[10];
};
struct UsrTable {
	char group[10][10];
	Usr usrs[10];
	int u_count = 0;
	int group_count = 0;
};





