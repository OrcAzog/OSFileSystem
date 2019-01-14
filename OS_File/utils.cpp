#include"utils.h"

string int2mod(int i) {
	string res = "";
	int a1, a2, a3;
	a1 = i / 100;
	a2 = (i / 10 - a1 * 10);
	a3 = i % 10;
	int num[] = { a1, a2, a3 };
	vector<int>vec({ a1, a2, a3 });
	for (auto i : vec) {
		if (i & 4)res += "r";
		else  res += "-";
		if (i & 2)res += "w";
		else  res += "-";
		if (i & 1)res += "x";
		else  res += "-";
	}
	return res;
}
int mod2int(string s) {
	int res = 0;
	for (int i = 0; i < 3; i++) {
		int temp = 0;
		if (s[i * 3 + 0] == 'r') {
			temp += 4;
		}
		if (s[i * 3 + 1] == 'w') {
			temp += 2;
		}
		if (s[i * 3 + 1] == 'x') {
			temp += 1;
		}
		res *= 10;
		res += temp;
	}
	return res;
}
int64_t getCurrentTime()      //直接调用这个函数就行了，返回值最好是int64_t，long long应该也可以
{
	time_t t;
	time(&t);
	return t;
}

string timeStampToString(int time) {
	time_t t;
	struct tm *p;
	t = time + 28800;
	p = gmtime(&t);
	char s[80];
	strftime(s, 80, "%Y-%m-%d %H:%M:%S", p);
	return string(s);
}
bool examMod(int a) {
	if (a > 777)return false;
	while (a != 0) {
		int temp = a % 10;
		a /= 10;
		if (temp > 7)return false;
	}
}
vector<string> simplifyPath(string path) {
	string res, temp;
	vector<string> stk;
	if (path[0] == '/') { stk.push_back("/"); }
	stringstream ss(path);
	while (getline(ss, temp, '/')) {
		if (temp == "." || temp == "") continue;
		else if (temp == "..") {
			if (!stk.empty() && stk[stk.size() - 1] != "..")
				stk.pop_back();
			else stk.push_back("..");
		}
		else if (temp != "..") stk.push_back(temp);
	}

	return stk;
}