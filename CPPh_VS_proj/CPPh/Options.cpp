#include "stdafx.h"
#include "Options.h"


Options::Options(int argc, char *argv[])
{
	for (int i = 0; i < argc; ){
		char **strend = 0; //temp foe strtoll and other sexy staff

		if (0 == strncmp(argv[i], "-p", strlen("-p"))){ //-p <token password>
			if (i + 1 < argc && argv[i + 1] != 0 && argv[i + 1][0] != '-'){
				this->opt_p = argv[i + 1];
				i++;
			}
			else {
				this->opt_p = this->opt_p_default;
			}
			//cout << "[DEBUG] opt_p = " << this->opt_p << endl;//DEBUG
		}
		if (0 == strncmp(argv[i], "-n", strlen("-n"))){ //-n <container name>
			if (i + 1 < argc && argv[i + 1] != 0 && argv[i + 1][0] != '-'){
				this->opt_n = argv[i + 1];
				i++;
			}
			//cout << "[DEBUG] opt_n = " << this->opt_n << endl;//DEBUG
		}
		if (0 == strncmp(argv[i], "-r", strlen("-r"))){ //-r <reader name>
			if (i + 1 < argc && argv[i + 1] != 0 && argv[i + 1][0] != '-'){
				this->opt_r = argv[i + 1];
				i++;
			}
			//cout << "[DEBUG] opt_r = " << this->opt_r << endl;//DEBUG
		}
		if (0 == strncmp(argv[i], "-i", strlen("-i"))){ //-i <input file name>
			if (i + 1 < argc && argv[i + 1] != 0 && argv[i + 1][0] != '-'){
				this->opt_i = argv[i + 1];
				i++;
			}
			//cout << "[DEBUG] opt_i = " << this->opt_i << endl;//DEBUG
		}
		if (0 == strncmp(argv[i], "-d", strlen("-d")) && strlen(argv[i]) == strlen("-d") ){ //DEBUG
			this->opt_d = true;
			//cout << "[DEBUG] opt_d = " << this->opt_d << endl;//DEBUG
		}
		if (0 == strncmp(argv[i], "-l", strlen("-l")) && strlen(argv[i]) == strlen("-l")){ //list containers
			this->opt_l = true;
			//cout << "[DEBUG] opt_l = " << this->opt_l << endl;//DEBUG
		}
		if (0 == strncmp(argv[i], "-dd", strlen("-dd")) && strlen(argv[i]) == strlen("-dd")){ //DEBUG
			this->opt_dd = true;
			this->opt_d = true;
			//cout << "[DEBUG] opt_dd = " << this->opt_dd << endl;//DEBUG
		}
		if (0 == strncmp(argv[i], "-s", strlen("-s"))){ //container to save
			if (i + 1 < argc && argv[i + 1] != 0 && argv[i + 1][0] != '-'){
				this->opt_s = atoi(argv[i + 1]);
				i++;
			}
			else {
				this->opt_s = -1;
			}
			//cout << "[DEBUG] opt_s = " << this->opt_s << endl;//DEBUG
		}
		if (0 == strncmp(argv[i], "-h", strlen("-h"))){ //print help
			this->printHelp();
		}
		i++;
	}
}

void Options::printHelp(){
	const int indent = 20;
	cout << "Options: " << endl;
	cout << setw(indent) << "-p <token pass>" << " - specify token password, default - \"" << this->opt_p_default << "\"" << endl;
	cout << setw(indent) << "-r <reader name>" << " - specify reader name, default - \""<<this->opt_r_default <<"\""<< endl;
	cout << setw(indent) << "-i <APDU file name>" << " - specify input file name in format of SmartcardSniffer (code.google.com/p/smartcard-sniffer/), default - \"" << this->opt_i_default << "\"" << endl;
	cout << setw(indent) << "-l" << " - list containers" << endl;
	cout << setw(indent) << "-n <container namr>" << " - dump contaier with the name, if not specified - dump with the same name, not work when all containers are dumped" << endl;
	cout << setw(indent) << "-s [<number>]" << " - dump container with id=<number>. If <number> is omitted all containers will be dumped." << endl;
	cout << setw(indent) << "-d" << " - debug output" << endl;
	//cout << setw(indent) << "-dd" << " - extended debug output" << endl;
	cout << setw(indent) << "-h" << " - print this help" << endl;
	this->opt_h = true;
}

void Options::showOpt(){
	const int indent = 20;
	cout << "Options: " << endl;
	cout << setw(indent) << "-p = " << this->opt_p << endl;
	cout << setw(indent) << "-r = " << this->opt_r << endl;
	cout << setw(indent) << "-i = " << this->opt_i << endl;
	cout << setw(indent) << "-l = " << this->opt_l << endl;
	cout << setw(indent) << "-s = " << this->opt_s << endl;
	cout << setw(indent) << "-d = " << this->opt_d << endl;
	//cout << setw(indent) << "-dd = " << this->opt_dd << endl;
}

Options::~Options()
{
}
