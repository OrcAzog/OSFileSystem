#include"PhysicalLayer.h"

fstream file;
string cur;
SuperBlock  superblock;
Block blocks[BLOCKSUM];//块区表	
Inode inodes[INODESUM];//节点区表
Usr *curUser;
bool blocksTable[BLOCKSUM];
UsrTable usrTable;
dentry rootDentry;
dentry* currentDentry;

char buff[1024];
//int umask;
//void 


void initBlockStack() {

	for (int i = 1; i <BLOCKSUM; i++) {
		if (i == 24 || i == 31)continue;//todo去掉位图
		realseEmptyBlock(i);
		
	}
}
void realseEmptyBlock(int i) {
	emptyBlockTable *table = &superblock.emptyblockTable;
	if (table->sum == 100) {
		emptyBlockTable * n = new emptyBlockTable();
		n->vals[0] = i;
		memcpy(&blocks[i].c, table, sizeof(emptyBlockTable));
		memcpy(&superblock.emptyblockTable, n, sizeof(emptyBlockTable));
	}
	else {
		table->vals[table->sum++] = i;
	}

}
void LoadSuperBlock() {
	file.open(FILE_CLASS, ios::in | ios::binary);
	read(file, superblock);
	for (int i = 1; i < BLOCKSUM; i++) {
		file.read(blocks[i].c, Block_Size);
	}
	file.close();
}

void LoadInodes() {
	file.open(INODE_FILE, ios::in | ios::binary);

	for (int i = 0; i < INODESUM; i++) {
		//file.read(blocks[i].c, 128);
		read(file, inodes[i]);
	}
	file.close();
}
void LoadBlockMap() {
	file.open(INODE_FILE, ios::in | ios::binary);

	for (int i = 1; i < INODESUM; i++) {
		file.read(blocks[i].c, 128);
		read(file, inodes[i]);
	}
	file.close();
}
int findEmptyNode() {
	for (int i = 0; i < INODESUM; i++) {
		if (inodes[i].isEmpty)return i;
	}
}
int findEmptyBlock() {
	for (int i = 0; i < BLOCKSUM; i++) {
		if (!blocksTable[i]) { blocksTable[i] = true; return i; }
	}
}

bool checkUser(Inode * inode, int tar) {

	int n;
	if (strcmp(curUser->u_name, inode->user) == 0)n = 3;
	else if (curUser->u_group == findGroup(inode->user))n = 2;
	else n = 1;
	int flag = inode->i_auth;
	for (int i = 1; i < n; i++) {
		flag /= 10;
	}
	flag %= 10;
	int t = tar & flag;
	if (t != 0)return true;
	else return false;

}

template <typename T>
void write(fstream &f, T &t) {
	f.write((char*)&t, sizeof(t));
}


template <typename T>
void read(fstream &f, T &t) {
	f.read((char*)&t, sizeof(t));
	//cout << buff << endl;

}
dentry* findDentry(dentry *d, vector<string>strs, bool flag = true) {
	if (strs.size() == 0)return NULL;
	string str = strs[0];
	if (str == ".") {
		strs.erase(strs.begin());//删除首节点
		for (auto i : d->children) {
			if (i->current->d_this.f_name == strs[0]) {
				strs.erase(strs.begin());
				return findDentry(d, strs, false);
			}
		}
	}
	else if (str == "..") { strs[0] = "."; return findDentry(d, strs); }
	else {
		if (flag)return findDentry(&rootDentry, strs);
		for (auto i : d->children) {
			if (i->current->d_this.f_name == strs[0]) {
				strs.erase(strs.begin());
				return findDentry(i, strs, false);
			}
		}
	}
}
int  findGroup(char * c) {
	for (int i = 0; i < usrTable.group_count; i++) {
		if (strcmp(c, usrTable.group[i]) == 0)return i;
	}
	return -1;
}

int getBlockInnode(int k, Inode * node) {
	if (k < 4)return node->i_sector[k];
	else if (k < 132) {
		int res;
		memcpy((char*)&res, blocks[node->i_sector[4]].c + (k - 4) * 4, 4);

		return res;
	}
	else {
		int res;
		memcpy((char*)&res, blocks[node->i_sector[5]].c + (k - 132) / 128, 4);

		memcpy((char*)&res, blocks[res].c + (k - res * 128) % 128, 4);
		return res;
	}

}
vector<int> getEmptyBlockVec(int n) {
	vector<int>res;
	for (int i = 0; i < n; i++)
		res.push_back(findEmptyBlock());
	return res;
}
void writeIntoNode(char *c, Inode * node, int size, bool reWrite) {

	char * cur = c;
	int  curSize = node->i_curByteSum;
	if (node->isEmpty&&curUser != NULL) {
		strcpy(node->user, curUser->u_name);
		strcpy(node->group, usrTable.group[curUser->u_group]);
	}
	else if (curUser == NULL) {
		strcpy(node->user, "root");
		strcpy(node->group, "root");
	}

	int ord = 777, auth = 0, tempMask = superblock.umask;
	if (node->i_isDir)ord = 666;
	for (int i = 0; i < 3; i++) {
		int a = ord % 10;
		int b = tempMask % 10;
		auth = pow(10, i)*(a ^ b);
		ord /= 10; tempMask /= 10;
	}
	node->i_auth = auth;
	node->LastChange = getCurrentTime();
	if (reWrite) {
		for (int i = 0; i < 6; i++) {
			if (node->i_sector[i] < 0)break;
			blocksTable[node->i_sector[i]] = false;
			node->i_sector[i] = -1;//todo间接未处理

		}
		node->isEmpty = true;
	}
	int targetSize;
	if (node->isEmpty) {
		node->isEmpty = false;
		targetSize = size;
		node->i_curByteSum = 0;
		curSize = 0;
	}
	targetSize = curSize + size;
	int targetEnd = (targetSize % Block_Size == 0 ? targetSize / Block_Size + 1 : targetSize / Block_Size + 1);
	int targetStart = node->i_curByteSum / Block_Size;
	if (curSize%Block_Size != 0)
	{
		int x = getBlockInnode(node->i_curByteSum / Block_Size, node);
		char *c_first = blocks[x].c + node->i_curByteSum%Block_Size;
		memcpy(c_first, cur, min(size, 1024 - node->i_curByteSum%Block_Size));//不知道有没有错
		cur += 1024 - node->i_curByteSum%Block_Size;
		//targetSize -= (1024 - node->i_curByteSum%Block_Size);
		targetStart += 1;
	}
	//新增
	else {
		vector<int>vec = getEmptyBlockVec(targetEnd - targetStart);
		for (int i = targetStart; i < targetEnd; i++) {

			int newBlock = vec[i - targetStart];
			memcpy(blocks[newBlock].c, cur, min(size, 1024));
			cur += 1024;
			size -= 1024;
			if (i < 4)node->i_sector[i] = newBlock;
			else if (i < 132) {
				if (node->i_sector[4] == -1) node->i_sector[4] = findEmptyBlock();

				memcpy(blocks[node->i_sector[4]].c + ((i - 4) % 128) * 4, &newBlock, 4);
			}
			else {
				if (node->i_sector[5] == 0) node->i_sector[5] = findEmptyBlock();
				memcpy(blocks[node->i_sector[5]].c + ((i - 4) % 128) * 4, &newBlock, 4);
			}

		}
	}
	node->i_curByteSum = targetSize;


}
void writeIntoNodeWithCheck(char *c, Inode * node, int size, bool reWrite) {
	if (!checkUser(node, 2)) {
		{cout << "无权限"; return; }
	}
	writeIntoNode(c, node, size, reWrite);
}
int readFromNode(Inode * node, int size , int start ) {

	if (size == -1)size = node->i_curByteSum;
	size = size > node->i_curByteSum ? node->i_curByteSum : size;

	memset(buff, 0, sizeof(buff));
	int targetSum = size;
	targetSum = min(1024, targetSum);


	int targetStart = start / 1024;
	int targetOffset = start % 1024;
	int firstSize, secondSize = 0;
	if (targetSum + targetOffset > 1024) { firstSize = 1024 - targetOffset; secondSize = targetSum - firstSize; }
	else firstSize = targetSum;
	targetSum += start;
	int x = getBlockInnode(targetStart, node);
	memcpy(buff, blocks[x].c + targetOffset, firstSize);
	x = getBlockInnode(targetStart + 1, node);
	if (secondSize != 0) { memcpy(buff + firstSize, blocks[x].c, secondSize); }
	return targetSum;
}
int readFromNodeWithCheck(Inode * node, int size = -1, int start = 0) {
	if (!checkUser(node, 4)) {
		{cout << "无权限"; return INT16_MAX; }
	}
	return readFromNode(node, size, start);
}

void save() {

	writeIntoNode((char*)&usrTable, &inodes[54], sizeof(usrTable), true);
	writeIntoNode((char*)blocksTable, &inodes[superblock.Sblock_bitmap_start], sizeof(blocksTable), true);
	file.open(FILE_CLASS, ios::out | ios::binary);
	write(file, superblock);
	for (int i = 1; i < BLOCKSUM; i++) {
		file.write(blocks[i].c, Block_Size);
	}

	file.close();
	file.open(INODE_FILE, ios::out | ios::binary);
	for (int i = 0; i < INODESUM; i++) {
		inodes[i].i_key = i;
		write(file, inodes[i]);
	}
	file.close();
}
dentry* buildDentry(FCB fcb, bool flag, dentry *d) {
	if (d == NULL) d = new dentry();
	dir *File = new dir();
	readFromNode(&inodes[fcb.inode], sizeof(*File));
	memcpy((char*)File, buff, sizeof(*File));

	d->current = File;
	d->str = string(File->d_this.f_name);

	if (!flag)return d;
	for (int i = 0; i < File->d_count; i++) {
		if (File->children[i].f_type != FT_DIRECTORY)continue;
		dentry * temp = buildDentry(File->children[i], false);
		d->children.push_back(temp);
		temp->father = d;
	}
	return d;

}
