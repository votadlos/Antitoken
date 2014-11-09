#include "utils.h"
#include <fstream>

void showarr(LPCBYTE b, DWORD n, char delim){
	for (int i = 0; i < n-1; i++)
		cout << setw(2) << setfill('0') << hex << (int)b[i] << delim;
	cout << setw(2) << setfill('0') << hex << (int)b[n-1] << endl;
}

void showarr2(LPCBYTE b, DWORD n){
	for (int i = 0; i < n; i++)
		cout << b[i];
	cout << endl;
}

void print2file(ofstream &f, LPCBYTE b, DWORD n){
	for (int i = 0; i < n; i++)
		f << setw(2) << setfill('0') << hex << (int)b[i];
	f << endl;
}

void split(string s, string delim, vector<string> &v){
	size_t pos = 0;
	v.empty();

	while ((pos = s.find(delim)) != string::npos) {
		v.push_back(s.substr(0, pos));
		s.erase(0, pos + delim.length());
	}
	v.push_back(s);
}

void hex2byte(vector<string> in, vector<BYTE> &out){
	out.empty();
	for (auto i = in.begin(); i != in.end(); i++){
		char **end = NULL;
		out.push_back( static_cast<BYTE>(strtol(i->data(), end, 16)) );
	}
}

ostream &operator<<(ostream &out, vector<string> &v){
	for (auto i = v.begin(); i != v.end(); i++){
		out << *i << " ";
	}
	return out;
}
ostream &operator<<(ostream &out, vector<BYTE> &v){
	for (auto i = v.begin(); i != v.end(); i++){
		//out << hex << setw(2) << (int)*i << " ";
		out << *i;
	}
	return out;
}