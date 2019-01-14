#pragma once
#include"PhysicalLayer.h"
#include"utils.h"

void mkdir(string s);
void doMkdir(dentry *fatherFile, vector<string>vec);
void init();
void mkfile(string s);
int find(dentry *d, string s);
void doCD(dentry *d, vector<string> s);
void pwd();
void ls();
Usr* findUser(string s);
void cat(string s );
void cd(string s);
void entryPoint();
int  findGroup(char * c);
string lsItem(FCB fcb);
void ls();
Usr* findUser(string s);
void cat(string s);
void chmod(string s);
void doSu(Usr* s, string pwd, bool flag);
void su(string s, bool flag);
void buildUserHome(string usrname);
