#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include"StringChanger.h"
#include<json\json.h>
#include<cstdio>
#include"Person.h"
#include"Trie.h"
#include<fstream>
#include <codecvt>
using namespace std;
const int maxn = 5000000;
Trie trie;
char s[maxn];
wstring load;
string spr[4] = { "北京","上海","天津","重庆" };
inline void init_getCounty(trie_node *fa, Json::Value& county) {
	trie_node *p = trie.insert(fa, county["name"].asString(), 2);
	int sz = county["streets"].size();
	for (int i = 0;i < sz;i++) {
		trie.insert(p, county["streets"][i].asString(), 3);
	}
}
inline void init_getCity(trie_node *fa, Json::Value& city) {
	trie_node *p = trie.insert(fa, city["name"].asString(), 1);
	int  sz = city["districts"].size();
	for (int i = 0;i < sz;i++) {
		init_getCounty(p, city["districts"][i]);
	}
}
inline void init_getProvice(Json::Value& provice) {
	trie_node *p = trie.insert(provice["name"].asString(), 0);
	int sz = provice["cities"].size();
	for (int i = 0;i < sz;i++) {
		init_getCity(p, provice["cities"][i]);
	}
}
inline void init() {
	load = StringChanger().StringToWstring("区道路街巷里弄胡同");
	fstream is;
	is.open("a.json");
	is >> s;
	is.close();
	char *p = s;
	while (*p != '{') p++;
	string json = StringChanger().Utf8ToGbk(p);
	Json::Reader reader;
	Json::Value root;
	while (!reader.parse(json, root));
	int sz = root["provinces"].size();
	for (int i = 0;i < sz;i++) {
		init_getProvice(root["provinces"][i]);
	}
	return;
}

//分离名字
inline void getName(Person &person, wstring &wts) {
	int l = (int)wts.size();
	for (int i = 0;i < l;i++) {
		if (wts[i] == ',') {
			person.name = StringChanger().WstringToString(wts.substr(0, i));
			wts.erase(0, i + 1);
			return;
		}
	}
}
//分离电话号码
inline void getPhone(Person &person, wstring &wts) {
	int l = (int)wts.size();
	int cnt = 0;
	for (int i = 0;i < l;i++) {
		if (wts[i] >= '0' && wts[i] <= '9') cnt++;
		else cnt = 0;
		if (cnt == 11) {
			wstring phone;
			phone.assign(wts, i - cnt + 1, cnt);
			wts = wts.erase(i - cnt + 1, cnt);
			person.phone = StringChanger().WstringToString(phone);
			break;
		}
	}
}
//获取后三级地址
inline bool isNum(wchar_t t) {
	return (t >= '0' && t <= '9');
}
inline bool isCH(wchar_t t) {
	return t > 255;
}
inline void getAdress(Person &person, wstring &wts) {
	bool flag = 0;
	int l = (int)wts.size();
	for (int i = l-1;i > 0;i--) {
		if (wts[i] == 21495 && !isCH(wts[i - 1])) {
			if (i < l-1 && wts[i+1] == 27004) continue;
			for (int j = i;j > 0;j--) {
				if (!isCH(wts[j]) && isCH(wts[j - 1])) {
					person.adress[4] = StringChanger().WstringToString(wts.substr(0, j));
					person.adress[5] = StringChanger().WstringToString(wts.substr(j, i - j + 1));
					if(i + 1 < l)
						person.adress[6] = StringChanger().WstringToString(wts.substr(i + 1, l - i - 1));
					return;
				}
			}
		}
	}
	for (int i = l - 1;i > 0;i--) {
		for (int j = 0;j < 6;j++) {
			if (wts[i] == load[j]) {
					person.adress[4] = StringChanger().WstringToString(wts.substr(0, i + 1));
					if(i+1 < l)
						person.adress[6] = StringChanger().WstringToString(wts.substr(i + 1, l - i - 1));
					return;
			}
		}
		if (wts[i] == load[8] && wts[i - 1] == load[7]) {
				person.adress[4] = StringChanger().WstringToString(wts.substr(0, i + 1));
				if(i+1 < l)
					person.adress[6] = StringChanger().WstringToString(wts.substr(i + 1, l - i - 1));
				return;
		}
	}
	person.adress[4].clear();
	person.adress[6] = StringChanger().WstringToString(wts);
}
//补全地址
bool fixAdress(Person& person, trie_node *ptr, int rank) {
	if (ptr == NULL) {
		if (rank == 3 && !person.adress[3].size()) return false;
		ptr = trie.getPtr(person.adress[rank], rank);
	}
	if (person.adress[rank].size()) {
		if (ptr->es[rank] != person.adress[rank]) return false;
	}
	if (!rank) {
		person.adress[0] = ptr->es[rank];
		return true;
	}
	for (auto i : ptr->father[rank]) {
		if (fixAdress(person, i, rank - 1)) {
			if (!person.adress[rank].size()) person.adress[rank] = ptr->es[rank];
			return true;
		}
	}
	return false;
}
/*
 * init()进行字典树初始化，会消耗较大的时间(大约7s)，并非死循环
 */
int main() {
	cout << "正在初始化字典树" << endl;
	init();
	cout << "runing" << endl;
	fstream is;
	is.open("1.txt");
	Json::Value res;
	string adr = "00";
	cout.imbue(locale("chs"));
	wcout.imbue(locale("chs"));
	while (is >> s) {
		Person person;
		char *p = s;
		while (*p < '1' || *p > '3') p++;
		int op = *p - '0';
		wstring wts = StringChanger().Utf8ToWstring(p+2);
		getName(person, wts);
		getPhone(person, wts);
		trie.seach(person, StringChanger().WstringToString(wts));
		for (int i = 0;i < 4;i++) {
			string tspr = spr[i] + "市";
			if (person.adress[0] == spr[i] || person.adress[1] == tspr) {
				person.adress[0] = spr[i];
				person.adress[1] = tspr;
			}
		}
		//cout << person.name << " " << person.phone << endl;
		Json::Value tmp;
		if (op == 3) fixAdress(person, NULL, 3);
		for (int i = 0;i < 4;i++) {
			if (person.adress[i] == "直辖市") person.adress[i].clear();
			tmp["地址"].append(person.adress[i]);
		}
		tmp["姓名"] = person.name;
		tmp["电话"] = person.phone;
		if (op == 1) {
			tmp["地址"].append(person.adress[4]);
		}
		else {
			wts = StringChanger().StringToWstring(person.adress[4]);
			getAdress(person, wts);
			for (int i = 4;i < 7;i++)
				tmp["地址"].append(person.adress[i]);
		}
		res["answer"].append(tmp);
	}
	is.close();
	is.open("2.txt");
	is.imbue(locale(is.getloc(), new codecvt_utf8<wchar_t, 0x10ffff, little_endian>));
	Json::FastWriter writer;
	is << writer.write(res) << endl;
	cout << "ok" << endl;
	is.close();
	return 0;
}