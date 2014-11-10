#ifndef _UTILS_H_
#define _UTILS_H_

#include <winscard.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdlib>


using namespace std;

void showarr(LPCBYTE b, DWORD n, char delim);
void print2file(ofstream &f, LPCBYTE b, DWORD n);
void split(string str, string delim, vector <string> &v);
void hex2byte(vector<string> in, vector<BYTE> &out);

ostream &operator<<(ostream &out, vector<string> &v);
ostream &operator<<(ostream &out, vector<BYTE> &v);

#endif