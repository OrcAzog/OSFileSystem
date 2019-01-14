#include"applicationLayer.h"

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
	blocksTable[0] = true;
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
	mkdir("home");

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
	writeIntoNode((char*)ch.c_str(), &inodes[fcb.inode], strlen(ch.c_str()), false);
	//memcpy(currentDentry, buildDentry(currentDentry->current->d_this, true), sizeof(*currentDentry));
	buildDentry(currentDentry->current->d_this, true, currentDentry);
}
int find(dentry *d, string s) {//查找当前目录文件
	for (auto i : d->current->children) {
		if (strcmp(s.c_str(), i.f_name) == 0) {
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
	else if (s.size() > 0 && s[0] == "..") {
		s.erase(s.begin());
		if (d->father != NULL)return doCD(d->father, s);
		else { cout << " No such file or directory"; return; }
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
string lsItem(FCB fcb) {
	string res;
	//res += fcb.f_name;
	//res += " ";
	res += inodes[fcb.inode].user;
	res += " ";
	res += int2mod(inodes[fcb.inode].i_auth).c_str();
	res += " ";
	res += timeStampToString(getCurrentTime());
	res += " ";
	return res;

}
void ls() {
	dir *curDir = currentDentry->current;
	for (int i = 0; i < curDir->d_count; i++) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);
		cout << lsItem(curDir->children[i]);
		if (curDir->children[i].f_type == FT_DIRECTORY)SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		else if (curDir->children[i].f_type == FT_REGULAR)SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		printf("%s", curDir->children[i].f_name);
		if (curDir->children[i].f_type == FT_DIRECTORY)cout << "/";
		cout << endl;
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY);
}
Usr* findUser(string s) {
	for (int i = 0; i < usrTable.u_count; i++) {
		if (strcmp(usrTable.usrs[i].u_name, s.c_str()) == 0) {
			return &usrTable.usrs[i];
		}
	}
	return NULL;
}

void cat(string s) {
	int node = find(currentDentry, s);
	if (node == -1) {
		cout << "无此文件";
		return;
	}
	//int node = currentDentry->current->children[i].inode;
	int start = 0;
	int size = inodes[node].i_curByteSum;
	while (start < size) {
		memset(buff, 1024, 0);
		start = readFromNodeWithCheck(&inodes[node], size, start);
		cout << buff;
	}

}
void chmod(string s) {

	int node = find(currentDentry, s);
	if (node == -1) {
		cout << "无此文件";
		return;
	}
	if (!checkUser(&inodes[node], 2)) {
		{cout << "无权限"; return; }
	}
	int i;
	cin >> i;
	if (!examMod(i)) { cout << "符号错误"; return; }
	inodes[node].i_auth = i;
}
void doSu(Usr* s, string pwd, bool flag) {

	if (flag || strcmp(pwd.c_str(), s->password) == 0) {
		curUser = s;
		cd("/home/" + string(s->u_name));
	}
	else {
		cout << "密码错误";
	}
}
void su(string s, bool flag) {
	Usr*u = findUser(s);
	if (u == NULL)cout << "无此用户" << endl;
	else {
		string pwd;
		if (!flag) {
			cout << "请输入密码" << endl;
			cin >> pwd;
		}
		doSu(u, pwd, flag);

	}
}
void cd(string s) {
	doCD(currentDentry, simplifyPath(s));
}
void buildUserHome(string usrname) {
	mkdir("/home/" + usrname);
}
void buildNewUser(string group, string u_name, string password) {
	for (int i = 0; i < usrTable.group_count; i++) {
		if (usrTable.group[i] == group.c_str()) {
			for (int j = 0; j < usrTable.u_count; j++) {
				if (usrTable.usrs[j].u_name == group.c_str()) {
					cout << "用户已存在" << endl;
					return;
				}
			}
			Usr* u = &usrTable.usrs[usrTable.u_count++];
			strcpy(u->password, password.c_str());
			strcpy(u->u_name, u_name.c_str());
			u->u_group = i;
			buildUserHome(u_name);
			//cout << "创建成功" << endl;
			return;

		}
	}
	Usr* u = &usrTable.usrs[usrTable.u_count++];
	strcpy(u->password, password.c_str());
	strcpy(u->u_name, u_name.c_str());
	strcpy(usrTable.group[usrTable.group_count], group.c_str());
	u->u_group = usrTable.group_count++;
	buildUserHome(u_name);
	//cout << "创建成功" << endl;
}