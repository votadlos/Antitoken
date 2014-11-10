//
// Command line options
//
#pragma once
#include <cstdlib>
#include <vector>
#include <cstring>
#include <iostream>
#include <iomanip>
//#include <tchar.h>

using namespace std;

class Options
{
public:
	Options(int argc, char *argv[]);
	~Options();
	void printHelp();
	void showOpt();
	
	//defaults
	char *opt_r_default = "Aladdin Token JC 0",
		*opt_p_default = "1234567890",
		*opt_i_default = "APDU_in.txt";

	//options
	bool opt_h = false;							//help
	bool opt_d = false;							//debug
	bool opt_dd = false;						//mega debug
	bool opt_l = false;							//list containers
	char *opt_n = 0;							//name of container to dump
	char *opt_p = 0;							//token password
	char *opt_r = this->opt_r_default;			//reader name
	char *opt_i = this->opt_i_default;			//input file name
	int opt_s = -10;							//number of container to save


};

