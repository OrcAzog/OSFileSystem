#include"源.cpp"
fstream file;
string cur = "/";
SuperBlock  superblock;
Block blocks[BLOCKSUM] = {};//块区表	
Inode inodes[INODESUM] = {};//节点区表
Usr *curUser;
bool blocksTable[BLOCKSUM] = { true };
UsrTable usrTable;
dentry rootDentry;
dentry* currentDentry;
char buff[2048] = {};
int size = 0;
void bulidUserHome(string usrname) {
	mkdir("/home/" + usrname);
}
void buildNewUser(string group, string u_name, string password) {
	for (int i = 0; i < usrTable.group_count; i++) {
		if (usrTable.group[i] == group.c_str()) {
			for (int j = 0; j < usrTable.u_count; j++) {
				if (usrTable.usrs[j].u_name == group.c_str()) {
					//cout << "用户已存在" << endl;
					return;
				}
			}
			Usr* u = &usrTable.usrs[usrTable.u_count++];
			strcpy(u->password, password.c_str());
			strcpy(u->u_name, u_name.c_str());
			u->u_group = i;
			bulidUserHome(u_name);
			//cout << "创建成功" << endl;
			return;

		}
	}
	Usr* u = &usrTable.usrs[usrTable.u_count++];
	strcpy(u->password, password.c_str());
	strcpy(u->u_name, u_name.c_str());
	strcpy(usrTable.group[usrTable.group_count], group.c_str());
	u->u_group = usrTable.group_count++;
	bulidUserHome(u_name);
	//cout << "创建成功" << endl;
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
int64_t getCurrentTime()      //直接调用这个函数就行了，返回值最好是int64_t，long long应该也可以
{
	time_t t;
	time(&t);
	return t;
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
	if (node->isEmpty&&curUser!=NULL) {
		strcpy(node->user, curUser->u_name);
		strcpy(node->group, usrTable.group[curUser->u_group]);
	}
	if (node->i_isDir)node->i_auth = 644;
	else node->i_auth = 755;
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
	 targetSize =curSize + size;
	int targetEnd = (targetSize % Block_Size == 0 ? targetSize / Block_Size+1 : targetSize / Block_Size+1) ;
	int targetStart = node->i_curByteSum / Block_Size;
	if (curSize%Block_Size != 0)
	{	
		int x = getBlockInnode(node->i_curByteSum / Block_Size, node);
		char *c_first = blocks[x].c + node->i_curByteSum%Block_Size;
		memcpy(c_first, cur, 512 - node->i_curByteSum%Block_Size);
		cur += 512 - node->i_curByteSum%Block_Size;
		//targetSize -= (512 - node->i_curByteSum%Block_Size);
		targetStart += 1;
	}
	//新增
	else {
		vector<int>vec = getEmptyBlockVec(targetEnd - targetStart);
		for (int i = targetStart; i < targetEnd; i++) {

			int newBlock = vec[i - targetStart];
			memcpy(blocks[newBlock].c, cur, 512);
			cur += 512;
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
int readFromNode(Inode * node, int size = -1, int start = 0) {

	if (size == -1)size = node->i_curByteSum;
	size = size > node->i_curByteSum ? node->i_curByteSum : size;

	memset(buff, 0, sizeof(buff));
	int targetSum = size;
	targetSum = min(2048, targetSum);
	targetSum += start;
	int targetStart = start / 512;
	int targetOffset = start % 512;
	int target = targetSum % 512 == 0 ? targetSum / 512 : targetSum / 512 + 1;
	//4 4+512/4 4+128+128*128
	for (int i = targetStart; i < target; i++) {
		//cout << blocks[node->i_sector[i]].c;
		if (i < 4)memcpy(buff + 512 * i, blocks[node->i_sector[i]].c + targetOffset, 512);
		else if (i < 132) {
			int index = getBlockInnode(i, node);
			memcpy(buff + 512 * (i-targetStart), blocks[index].c + targetOffset, 512);
		}
		else {
			int temp;
			memcpy((char*)&temp, blocks[node->i_sector[5]].c + (i - 132) / 128, 4);
			
			memcpy((char*)&temp, blocks[temp].c + (i - temp * 128) % 128, 4);
			memcpy(buff + 512 * (i - targetStart), blocks[temp].c + targetOffset, 512);
		}
		targetOffset = 0;
	}
	return targetSum ;
}
template<class T>
int buildNewFile(T &c, int size, int val = -1) {
	if (val <= 0)
		val = findEmptyNode();
	Inode *node = &inodes[val];
	//cout << &c;
	//cout << buff;
	writeIntoNode((char*)&c, node, size, false);
	return val;
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
dentry* buildDentry(FCB fcb, bool flag, dentry *d = NULL) {
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
void Load() {
	LoadSuperBlock();
	LoadInodes();
	dir baseFile;

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
void mkdir(string s) {
	dentry * temp = currentDentry;
	vector<string>vec = simplifyPath(s);
	doMkdir(currentDentry, vec);
	currentDentry = temp;
}
void doMkdir(dentry *fatherFile, vector<string>vec) {
	string name = vec[vec.size() - 1];
	vec.pop_back();
	doCD(fatherFile, vec);
	fatherFile = currentDentry;
	dir targetDir, *fatherDir = currentDentry->current;
	FCB *fcb = &targetDir.d_this, *F_fcb = &fatherFile->current->d_this;

	dentry *tempDentry = new dentry();

	fcb->f_type = FT_DIRECTORY;

	strcpy(fcb->f_name, name.c_str());
	fcb->inode = findEmptyNode();
	targetDir.father = &currentDentry->current->d_this;
	fatherDir->children[fatherDir->d_count++] = targetDir.d_this;
	writeIntoNode((char*)fatherDir, &inodes[F_fcb->inode], sizeof(*fatherDir), true);
	inodes[fcb->inode].i_isDir = true;
	writeIntoNode((char*)&targetDir, &inodes[fcb->inode], sizeof(targetDir), false);

	buildDentry(currentDentry->current->d_this, true, currentDentry);
}
void init() {
	dir baseFile;
	baseFile.d_this.f_type = FT_DIRECTORY;
	FCB*fcb = &baseFile.d_this;
	strcpy(fcb->f_name, "/");
	fcb->inode = findEmptyNode();//0
	inodes[fcb->inode].i_isDir = true;
	//baseFile.dir_name = "";
	buildNewFile(baseFile, sizeof(baseFile), fcb->inode);
	//baseFile 
	buildNewFile(blocksTable, sizeof(blocksTable), superblock.Sblock_bitmap_start);

	save();

	Load();

	//strcpy(buff, "home");
	mkdir("home");
	//strcpy(buff, "root");
	//mkdir("root");
	//cd("/home");
	//strcpy(buff, "vistor");
	//mkdir("/home/vistor");
	buildNewUser("root", "root", "root");
	buildNewUser("vistor", "vistor", "vistor");

	save();



}

void mkfile(string s) {
	dentry  *fatherFile = currentDentry;
	dir  *fatherDir = currentDentry->current;
	FCB fcb, *F_fcb = &fatherFile->current->d_this;

	dentry *tempDentry = new dentry();

	fcb.f_type = FT_REGULAR;;
	strcpy(fcb.f_name, s.c_str());
	fcb.inode = findEmptyNode();
	//char ch[100] = "4556";
	string ch;
	cin >> ch;
	fatherDir->children[fatherDir->d_count++] = fcb;
	writeIntoNode((char*)fatherDir, &inodes[F_fcb->inode], sizeof(*fatherDir), true);
	//writeIntoNode((char*)&targetDir, &inodes[fcb->inode], sizeof(targetDir), false);
	writeIntoNode((char*)ch.c_str(), &inodes[fcb.inode], sizeof(ch.c_str()), false);
	//memcpy(currentDentry, buildDentry(currentDentry->current->d_this, true), sizeof(*currentDentry));
	buildDentry(currentDentry->current->d_this, true, currentDentry);
}
int find(dentry *d, string s) {//查找当前目录文件
	for (auto i : d->current->children) {
		if (strcmp(s.c_str(), i.f_name)) {
			return i.inode;
		}
	}
	return -1;
}
void doCD(dentry *d, vector<string> s) {
	if (s.size() > 0 && s[0] == "/") {
		d = &rootDentry;
		s.erase(s.begin());
	}
	buildDentry(d->current->d_this, true, d);
	dentry *cur_Dentry = d;
	if (s.size() == 0) {
		currentDentry = d;

		return;
	}
	for (int i = 0; i < d->children.size(); i++) {
		if (d->children[i]->current->d_this.f_name == s[0]) {
			cur_Dentry = cur_Dentry->children[i];
			buildDentry(cur_Dentry->current->d_this, true, cur_Dentry);
			s.erase(s.begin());
			doCD(cur_Dentry, s);
			return;
		}
	}
	cout << " No such file or directory";

}
void pwd() {
	vector<string>vec;
	dentry *d = currentDentry;
	while (d != NULL) {
		vec.push_back(d->current->d_this.f_name);
		d = d->father;
	}
	for (int i = vec.size() - 1; i >= 0; i--) {
		cout << vec[i];
		if (i != 0 && i != vec.size() - 1)cout << "/";
	}
}
void ls() {
	dir *curDir = currentDentry->current;
	for (int i = 0; i < curDir->d_count; i++) {
		if (curDir->children[i].f_type == FT_DIRECTORY)SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN |
			FOREGROUND_BLUE |
			FOREGROUND_INTENSITY);
		else if (curDir->children[i].f_type == FT_REGULAR)SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		printf("%s\t", curDir->children[i].f_name);
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);
}
vector<string> simplifyPath(string path) {
	string res, temp;
	vector<string> stk;
	if (path[0] == '/') { stk.push_back("/"); }
	stringstream ss(path);
	while (getline(ss, temp, '/')) {
		if (temp == "." || temp == "") continue;
		else if (temp == ".." && !stk.empty()) stk.pop_back();
		else if (temp != "..") stk.push_back(temp);
	}

	return stk;
}
Usr* findUser(string s) {
	for (int i = 0; i < usrTable.u_count; i++) {
		if (strcmp(usrTable.usrs[i].u_name, s.c_str())) {
			return &usrTable.usrs[i];
		}
	}
	return NULL;
}
void cat(string s, string s2 = NULL) {
	for (auto i : currentDentry->children) {
		if (i->current->d_this.f_name == s.c_str()) {}
	}
}
void su(string s, string p) {
	Usr* temp = findUser(s);
	if (temp == NULL) {
		cout << "无此用户";
		return;
	}
	else if (temp->password == p.c_str()) {
		curUser = temp;
	}
	else {
		cout << "密码错误";
		return;
	}

}
void cd(string s) {
	doCD(currentDentry, simplifyPath(s));
}
void entryPoint() {
	Load();
	string s;
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
		else {

		}
		cout << endl;
	}
}
void main() {
	//entryPoint();
	int x = findEmptyNode();
	for (int i = 0; i < 20; i++) {
		char c = 'a';
		writeIntoNode(&c, &inodes[x], 1, false);
	}
	int start=readFromNode(&inodes[x], 5);
	int len = 0;
	while (start<4) {
	//	cout << strlen(buff)<<endl;
		len += strlen(buff);
		cout << buff;
		start = readFromNode(&inodes[x], 5, start);

	}
	len += strlen(buff);
	cout << len;
	//cout << strlen(buff);
}

