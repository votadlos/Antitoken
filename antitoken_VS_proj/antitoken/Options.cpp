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
		if (0 == strncmp(argv[i], "-k", strlen("-k"))){ //-k <registry key name>
			if (i + 1 < argc && argv[i + 1] != 0 && argv[i + 1][0] != '-'){
				this->opt_k = argv[i + 1];
				i++;
			}
			//cout << "[DEBUG] opt_k = " << this->opt_k << endl;//DEBUG
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
	cout << setw(indent) << "-k <registry key>" << " - registry key path with encrypted password, usually smth like \"SOFTWARE\\Crypto Pro\\Settings\\USERS\\S-1-5-21-742138110-4119056388-1046353323-500\\KeyDevices\\passwords\\SCARD\\ETOKEN_JAVA_01c3096e\\CC00\" from HKEY_LOCAL_MACHINE" << endl;
	cout << setw(indent) << "-i <APDU file name>" << " - specify input file name in format of SmartcardSniffer (code.google.com/p/smartcard-sniffer/), default - \"" << this->opt_i_default << "\"" << endl;
	cout << setw(indent) << "-l" << " - list containers" << endl;
	cout << setw(indent) << "-n <container name>" << " - dump contaier with the name, if not specified - dump with the same name, but with the first letter changed to 'N' or 'M'" << endl;
	cout << setw(indent) << "-s [<number>]" << " - dump container with id=<number>. If <number> is omitted all containers will be dumped." << endl;
	cout << setw(indent) << "-d" << " - debug output" << endl;
	//cout << setw(indent) << "-dd" << " - extended debug output" << endl;
	cout << setw(indent) << "-h" << " - print this help" << endl;
	this->opt_h = true;
	cout << endl<<"Common use cases: " << endl << "> antitoken.exe -l == List all containers on token" << endl
		<< "> antitoken.exe -s 5 -p == dump container with id 5 from token with default password" << endl
		<< "> antitoken.exe -s 5 -p qwerty == dump container with id 5 from token with password 'qwerty' " << endl
		<< "> antitoken.exe -s 5 -p qwerty -n asdfrt56 == dump container with id 5 from token with password 'qwerty' and save it with the name 'asdfrt56'" << endl
		<< "> antitoken.exe -s -p qwerty == dump all containers from token with password 'qwerty'" << endl
		<< "> antitoken.exe -k \"SOFTWARE\\Crypto Pro\\Settings\\USERS\\S-1-5-21-742138110-4119056388-1046353323-500\\KeyDevices\\passwords\\SCARD\\ETOKEN_JAVA_01c3096e\\CC00\" == dump cached by Crypto-Pro token password" << endl
		<< "> antitoken.exe -i APDU_list.txt == read APDU commands from file and send them to token, APDU_list.txt can be omited - default will be taken" << endl;
}

void Options::showOpt(){
	const int indent = 20;
	cout << "Options: " << endl;
	if (this->opt_p) cout << setw(indent) << "-p = " << this->opt_p << endl;
	if (this->opt_k) cout << setw(indent) << "-k = " << this->opt_k << endl;
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
