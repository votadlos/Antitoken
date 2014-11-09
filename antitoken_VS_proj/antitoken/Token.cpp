#include "Token.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

#include "sha.h"//crypto++
#include "des.h"//crypto++
#include "modes.h"//crypto++


Token::Token(Options opt)
{
	LONG            lReturn;
	DWORD           dwAP;

	this->debug = opt.opt_d;
	memset(this->upass, 0, this->lpass);
	if (opt.opt_p) memcpy(this->upass, opt.opt_p, strlen(opt.opt_p));

	memset(this->apass, 0, this->lpass);

	memset(this->name, 0, this->lname);
	if (opt.opt_n) memcpy(this->name, opt.opt_n, strlen(opt.opt_n));

	memset(this->reader, 0, this->lreader);
	memcpy(this->reader, opt.opt_r, strlen(opt.opt_r));

	memset(this->ATR, 0, this->lATR);



	// Establish the context.
	lReturn = SCardEstablishContext(SCARD_SCOPE_USER,
		NULL,
		NULL,
		&(this->hSC) );
	if (SCARD_S_SUCCESS != lReturn){
		cout << "Failed SCardEstablishContext: " << lReturn << endl;
		return;
	}
	if (opt.opt_d) cout << "[*] Context established" << endl;

	//connect to SmartCard
	lReturn = SCardConnect(hSC,
		(LPCTSTR)this->reader,
		SCARD_SHARE_SHARED,
		SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
		//SCARD_PROTOCOL_T0,
		&(this->hCardHandle),
		&dwAP);
	if (SCARD_S_SUCCESS != lReturn)
	{
		cout << "[*] ERROR: Failed SCardConnect: " << lReturn << endl;
		return;
	}
	if (opt.opt_d) cout << "[*] Connection established" << endl;

	// Use the connection.
	// Display the active protocol.
	switch (dwAP)
	{
	case SCARD_PROTOCOL_T0:
		if (opt.opt_d) cout << "[*] Active protocol T0" << endl;
		break;

	case SCARD_PROTOCOL_T1:
		if (opt.opt_d) cout << "[*] Active protocol T1" << endl;
		break;

	case SCARD_PROTOCOL_UNDEFINED:
	default:
		if (opt.opt_d) cout << "[*] Active protocol unnegotiated or unknown" << endl;
		break;
	}

}


Token::~Token()
{
	LONG            lReturn;

	// Desconnect
	lReturn = SCardDisconnect(hCardHandle,
		SCARD_LEAVE_CARD);
	if (SCARD_S_SUCCESS != lReturn)
	{
		cout << "Failed SCardDisconnect" << endl;
	}

	//Free context
	lReturn = SCardReleaseContext(hSC);
	if (SCARD_S_SUCCESS != lReturn) {
		cout << "Failed SCardReleaseContext" << endl;
	}
}

bool Token::checkATR(){
	LONG lReturn;
	char szReader[200];
	DWORD cch = 200;
	DWORD dwState, dwProtocol;

	lReturn = SCardStatus(hCardHandle, szReader, &cch, &dwState, &dwProtocol, (LPBYTE)&(this->ATR), &(this->lATR));
	if (this->lATR == laladdinATR && memcmp(this->ATR, aladdinATR, this->lATR) == 0){
		return true;
	}
	else {
		return false;
	}
}

void Token::list_containers(vector<string> &res){
	LONG lReturn;
	BYTE cont_number = 0;
	BYTE *pbSend, *pbRecv;
	DWORD dwRecv;
	const DWORD  cdwRecv = 1024;
	const DWORD cdwSend = 128;

	res.clear();

	pbSend = new BYTE[cdwSend];
	pbRecv = new BYTE[cdwRecv];

	//begin transaction
	lReturn = SCardBeginTransaction(this->hCardHandle);
	if (SCARD_S_SUCCESS != lReturn){
		cout << "[*] ERROR: Failed SCardBeginTransaction: " << lReturn << endl;
		return;
	}

	while (true){
		this->sendPreambula();

		memset(pbSend, 0, cdwSend);
		memcpy(pbSend, s4, 16);
		pbSend[14] = cont_number;
		dwRecv = cdwRecv;
		SCardTransmit(this->hCardHandle, SCARD_PCI_T1, pbSend, 16, NULL, pbRecv, &dwRecv); //send s4 with container number
		if (memcmp(pbRecv, noFile, 2) == 0) //file not found
			break;

		memset(pbSend, 0, cdwSend);
		memcpy(pbSend, s5, 18);
		pbSend[14] = cont_number;
		dwRecv = cdwRecv;
		SCardTransmit(this->hCardHandle, SCARD_PCI_T1, pbSend, 18, NULL, pbRecv, &dwRecv); //send s5 with container number

		dwRecv = cdwRecv;
		SCardTransmit(this->hCardHandle, SCARD_PCI_T1, cmdGIVE_10, 10, NULL, pbRecv, &dwRecv); //claim first 10 bytes of container name
		DWORD container_name_len = pbRecv[3];
		string container_name;
		int i = 4;
		while (pbRecv[i] != 0x00 && pbRecv[i] != 0x90){
			container_name.append((char *)(pbRecv + i), 1);
			i++;
		}

		if (container_name_len > 6){ //received 10b without header with length in 4th byte
			memset(pbSend, 0, cdwSend);
			memcpy(pbSend, cmdGIVE_10, 10);
			pbSend[8] = 0x0a; //10b that we received
			pbSend[9] = container_name_len - pbSend[8] + 4; //the rest
			dwRecv = cdwRecv;
			SCardTransmit(this->hCardHandle, SCARD_PCI_T1, pbSend, 10, NULL, pbRecv, &dwRecv);
			container_name.append((char *)pbRecv, dwRecv - 2);
		}

		res.push_back(container_name);

		cont_number++;
	}

	//End transaction
	SCardEndTransaction(this->hCardHandle, SCARD_LEAVE_CARD);
	
	delete[] pbRecv;
	delete[] pbSend;
}

int Token::dump_header(vector<BYTE> &res, DWORD id){
	
	LONG lReturn;
	BYTE cont_number = 0;
	BYTE *pbSend, *pbRecv;
	DWORD dwRecv;
	const DWORD  cdwRecv = 1024;
	const DWORD cdwSend = 128;

	res.clear();

	pbSend = new BYTE[cdwSend];
	pbRecv = new BYTE[cdwRecv];

	//begin transaction
	lReturn = SCardBeginTransaction(this->hCardHandle);
	if (SCARD_S_SUCCESS != lReturn){
		cout << "[*] ERROR: Failed SCardBeginTransaction: " << lReturn << endl;
		return 20;
	}

	this->sendPreambula();

	memset(pbSend, 0, cdwSend);
	memcpy(pbSend, s6, ls6);
	pbSend[14] = id;
	dwRecv = cdwRecv;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, pbSend, ls6, NULL, pbRecv, &dwRecv); //send s6 with container id
	if (memcmp(pbRecv, noFile, 2) == 0){ //file not found
		delete[] pbRecv;
		delete[] pbSend;

		return 10; //no such container id
	}
	//cout << ">>>(s6) "; showarr(pbSend, ls6, ':'); //DEBUG
	//cout << "<<< "; showarr(pbRecv, dwRecv, ':'); //DEBUG


	dwRecv = cdwRecv;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, cmdGIVE_10, lcmdGIVE_10, NULL, pbRecv, &dwRecv); //claim first 10 bytes of header.key
	for (int i = 0; i < 0x0a; i++) res.push_back(pbRecv[i]);
	//cout << ">>>(1st 10) "; showarr(cmdGIVE_10, lcmdGIVE_10, ':'); //DEBUG
	//cout << "<<< "; showarr(pbRecv, dwRecv, ':'); //DEBUG

	DWORD header_len = (pbRecv[2] << 8) + pbRecv[3]; //length of header
	DWORD bytes_received = 0x0a; //now received
	const DWORD portion = 0xf5; //bytes claim with one request

	while (bytes_received < header_len){
		memset(pbSend, 0, cdwSend);
		memcpy(pbSend, cmdGIVE_10, lcmdGIVE_10);
		pbSend[7] = bytes_received >> 8;
		pbSend[8] = bytes_received & 0xff;
		pbSend[9] = ((header_len - bytes_received + 4) > portion) ? portion : (header_len - bytes_received + 4);
		dwRecv = cdwRecv;
		SCardTransmit(this->hCardHandle, SCARD_PCI_T1, pbSend, lcmdGIVE_10, NULL, pbRecv, &dwRecv);
		bytes_received += pbSend[9];
		for (int i = 0; i < pbSend[9]; i++) res.push_back(pbRecv[i]);
		//cout << ">>> "; showarr(pbSend, lcmdGIVE_10, ':'); //DEBUG
		//cout << "<<< "; showarr(pbRecv, dwRecv, ':'); //DEBUG
	}

	//End transaction
	SCardEndTransaction(this->hCardHandle, SCARD_LEAVE_CARD);

	delete[] pbRecv;
	delete[] pbSend;

	//cout << res << endl; //DEBUG

	return 0;
}

int Token::dump_name(vector<BYTE> &res, DWORD id){
	
	LONG lReturn;
	BYTE cont_number = 0;
	BYTE *pbSend, *pbRecv;
	DWORD dwRecv;
	const DWORD  cdwRecv = 1024;
	const DWORD cdwSend = 128;

	res.clear();

	pbSend = new BYTE[cdwSend];
	pbRecv = new BYTE[cdwRecv];

	//begin transaction
	lReturn = SCardBeginTransaction(this->hCardHandle);
	if (SCARD_S_SUCCESS != lReturn){
		cout << "[*] ERROR: Failed SCardBeginTransaction: " << lReturn << endl;
		return 20;
	}

	this->sendPreambula();

	memset(pbSend, 0, cdwSend);
	memcpy(pbSend, s5, ls5);
	pbSend[14] = id;
	dwRecv = cdwRecv;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, pbSend, ls6, NULL, pbRecv, &dwRecv); //send s6 with container id
	if (memcmp(pbRecv, noFile, 2) == 0){ //file not found
		delete[] pbRecv;
		delete[] pbSend;

		return 10; //no such container id
	}
	//cout << ">>>(s5) "; showarr(pbSend, ls5, ':'); //DEBUG
	//cout << "<<< "; showarr(pbRecv, dwRecv, ':'); //DEBUG

	dwRecv = cdwRecv;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, cmdGIVE_ALL_name, lcmdGIVE_ALL_name, NULL, pbRecv, &dwRecv); //claim first 10 bytes of header.key
	for (int i = 0; i < dwRecv - 2 && pbRecv[i] != 0x00; i++) res.push_back(pbRecv[i]);
	//cout << ">>>(1st 10) "; showarr(cmdGIVE_ALL_name, lcmdGIVE_ALL_name, ':'); //DEBUG
	//cout << "<<< "; showarr(pbRecv, dwRecv, ':'); //DEBUG

	//End transaction
	SCardEndTransaction(this->hCardHandle, SCARD_LEAVE_CARD);

	delete[] pbRecv;
	delete[] pbSend;

	return 0;
}

int Token::dump_masks(vector<BYTE> &res, DWORD id){

	LONG lReturn;
	BYTE cont_number = 0;
	BYTE *pbSend, *pbRecv;
	DWORD dwRecv;
	const DWORD  cdwRecv = 1024;
	const DWORD cdwSend = 128;

	// need to login
	if (0 != this->et72k_Login()) return 11;

	res.clear();

	pbSend = new BYTE[cdwSend];
	pbRecv = new BYTE[cdwRecv];

	//begin transaction
	lReturn = SCardBeginTransaction(this->hCardHandle);
	if (SCARD_S_SUCCESS != lReturn){
		cout << "[*] ERROR: Failed SCardBeginTransaction: " << lReturn << endl;
		return 20;
	}

	memset(pbSend, 0, cdwSend);
	memcpy(pbSend, s7, ls7);
	pbSend[14] = id;
	dwRecv = cdwRecv;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, pbSend, ls7, NULL, pbRecv, &dwRecv); //send s7 with container id
	if (memcmp(pbRecv, noFile, 2) == 0){ //file not found
		delete[] pbRecv;
		delete[] pbSend;

		return 10; //no such container id
	}
	//cout << ">>>(s7) "; showarr(pbSend, ls7, ':'); //DEBUG
	//cout << "<<< "; showarr(pbRecv, dwRecv, ':'); //DEBUG


	dwRecv = cdwRecv;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, cmdGET_MASKS, lcmdGET_MASKS, NULL, pbRecv, &dwRecv); //claim masks.key
	for (int i = 0; i < dwRecv - 2; i++) res.push_back(pbRecv[i]);
	//cout << ">>> "; showarr(cmdGET_MASKS, lcmdGET_MASKS, ':'); //DEBUG
	//cout << "<<< "; showarr(pbRecv, dwRecv, ':'); //DEBUG

	//End transaction
	SCardEndTransaction(this->hCardHandle, SCARD_LEAVE_CARD);

	delete[] pbRecv;
	delete[] pbSend;

	return 0;
}


int Token::dump_primary(vector<BYTE> &res, DWORD id){

	LONG lReturn;
	BYTE cont_number = 0;
	BYTE *pbSend, *pbRecv;
	DWORD dwRecv;
	const DWORD  cdwRecv = 1024;
	const DWORD cdwSend = 128;

	// need to login
	if (0 != this->et72k_Login()) return 11;

	res.clear();

	pbSend = new BYTE[cdwSend];
	pbRecv = new BYTE[cdwRecv];

	//begin transaction
	lReturn = SCardBeginTransaction(this->hCardHandle);
	if (SCARD_S_SUCCESS != lReturn){
		cout << "[*] ERROR: Failed SCardBeginTransaction: " << lReturn << endl;
		return 20;
	}

	memset(pbSend, 0, cdwSend);
	memcpy(pbSend, s11, ls11);
	pbSend[14] = id;
	dwRecv = cdwRecv;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, pbSend, ls11, NULL, pbRecv, &dwRecv); //send s11 with container id
	if (memcmp(pbRecv, noFile, 2) == 0){ //file not found
		delete[] pbRecv;
		delete[] pbSend;

		return 10; //no such container id
	}
	//cout << ">>>(s11) "; showarr(pbSend, ls11, ':'); //DEBUG
	//cout << "<<< "; showarr(pbRecv, dwRecv, ':'); //DEBUG


	dwRecv = cdwRecv;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, cmdGET_PRIMARY, lcmdGET_PRIMARY, NULL, pbRecv, &dwRecv); //claim primary.key
	for (int i = 0; i < dwRecv - 2; i++) res.push_back(pbRecv[i]);
	//cout << ">>> "; showarr(cmdGET_MASKS, lcmdGET_MASKS, ':'); //DEBUG
	//cout << "<<< "; showarr(pbRecv, dwRecv, ':'); //DEBUG

	//End transaction
	SCardEndTransaction(this->hCardHandle, SCARD_LEAVE_CARD);

	delete[] pbRecv;
	delete[] pbSend;

	return 0;
}

int Token::dump_container(DWORD id){
	int ret;
	vector<BYTE> v;
	char path[260],bid[20];
	
	char *header = "header.key",
		*masks = "masks.key",
		*primary = "primary.key",
		*name = "name.key",
		*pos;

	if ((ret = this->dump_name(v, id)) != 0){
		if (this->debug)
			cout << "[-] ERROR: dump_name returned: " << ret << endl;
		return ret;
	}
	memset(path, 0, 260);
	memset(bid, 0, 20);
	
	BYTE *tname = v.data();
	strncpy(path, (char *)(tname + 4), v.size() > 8 ? 4 : v.size() - 4);
	strcat(path, ".");
	strcat(path, itoa(id, bid, 10));
	CreateDirectory(path, NULL);
	if (this->debug) cout << "[*] CreateDirectory: " << path << endl; //DEBUG
	strcat(path, "\\");
	pos = path + strlen(path);

	strcpy(pos, name);
	ofstream name_key(path, ios::out | ios::binary);
	if (strlen(this->name) > 0){ // name specified
		int len = strlen(this->name);
		BYTE *b = new BYTE[len + 4];
		b[0] = 0x30;
		b[1] = len + 2;
		b[2] = 0x16;
		b[3] = len;
		for (int i = 0; i < len; i++)
			b[4 + i] = this->name[i];
		name_key << b;
		delete[] b;
	}
	else {
		if (v[4] == 'N') v[4] = 'M';
		else v[4] = 'N';
		name_key << v;
	}
	name_key.close();
	if(this->debug) cout << "[*] write name: " << path << endl; //DEBUG
	memset(pos, 0, strlen(name) + 1);

	strcpy(pos, header);
	ofstream header_key(path, ios::out | ios::binary);
	if ((ret = this->dump_header(v, id)) != 0){
		if (this->debug)
			cout << "[-] ERROR: dump_header returned: " << ret << endl;
		return ret;
	}
	header_key << v;
	header_key.close();
	if (this->debug) cout << "[*] write header: " << path << endl; //DEBUG
	memset(pos, 0, strlen(header) + 1);

	strcpy(pos, masks);
	ofstream masks_key(path, ios::out | ios::binary);
	if ((ret = this->dump_masks(v, id)) != 0){
		if (this->debug)
			cout << "[-] ERROR: dump_masks returned: " << ret << endl;
		return ret;
	}
	masks_key << v;
	masks_key.close();
	if (this->debug) cout << "[*] write masks: " << path << endl; //DEBUG
	memset(pos, 0, strlen(masks) + 1);
	
	strcpy(pos, primary);
	ofstream primary_key(path, ios::out | ios::binary);
	if ((ret = this->dump_primary(v, id)) != 0){
		if (this->debug)
			cout << "[=] ERROR: dump_primary returned: " << ret << endl;
		return ret;
	}
	primary_key << v;
	primary_key.close();
	if (this->debug) cout << "[*] write primary: " << path << endl; //DEBUG
	memset(pos, 0, strlen(primary) + 1);

	return 0;
}

void Token::sendPreambula(){

	BYTE *pbRecv;
	DWORD dwRecv;
	const DWORD  cdwRecv = 1024;

	pbRecv = new BYTE[cdwRecv];

	dwRecv = cdwRecv;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, s1, ls1, NULL, pbRecv, &dwRecv); //send s1
	dwRecv = cdwRecv;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, s2, ls2, NULL, pbRecv, &dwRecv); //send s2
	dwRecv = cdwRecv;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, s3, ls3, NULL, pbRecv, &dwRecv); //send s3
	//cout << ">>>(s3) "; showarr(s3, ls3, ':'); //DEBUG
	//cout << "<<< "; showarr(pbRecv, dwRecv, ':'); //DEBUG

	delete[] pbRecv;

}

void Token::coumpute_authcode(BYTE challenge[8], BYTE rte_data[20]){
	BYTE static_rte64[64],
		response[8], //result
		data[64],
		rte_data64[64],
		digest[20],
		key_3des_24[24],
		magic_sum_out64[64];

	int pass_len = strlen(this->upass),
		pass_len_u = 2 * (pass_len + 1); //in UTF-8

	BYTE *token_pass_u = new BYTE[pass_len_u];
	memset(token_pass_u, 0, pass_len_u);

	for (int i = 0; i < pass_len; i++){
		token_pass_u[i * 2] = 0;
		token_pass_u[i * 2 + 1] = this->upass[i];
	}
	token_pass_u[pass_len * 2] = token_pass_u[pass_len * 2 + 1] = 0;
	//cout << "token_pass_u "; showarr(token_pass_u, pass_len_u, ':'); //DEBUG

	for (int i = 0; i < 64; i++){
		static_rte64[i] = 0x03;
		data[i] = token_pass_u[i % pass_len_u];
		rte_data64[i] = rte_data[i % 20];
	}

	//cout << "rte_data64 "; showarr(rte_data64, 64, ':'); //DEBUG
	//cout << "static_rte64 "; showarr(static_rte64, 64, ':'); //DEBUG
	//cout << "data "; showarr(data, 64, ':'); //DEBUG

	CryptoPP::SHA1 h;
	h.Update(static_rte64, 64);
	h.Update(rte_data64, 64);
	h.Update(data, 64);
	h.Final(digest);

	//cout << "1st digest "; showarr(digest, 20, ':'); //DEBUG

	for (int i = 0; i < 998; i++){
		CryptoPP::SHA1 h_cycle;
		h_cycle.Update(digest, 20);
		h_cycle.Final(digest);
	}

	memcpy(key_3des_24, digest, 20); //now we have 20 bytes of key, we need 4 bytes more

	h.Restart();
	h.Update(static_rte64, 64);

	this->magic_sum64(magic_sum_out64, digest, rte_data64);
	h.Update(magic_sum_out64, 64);

	this->magic_sum64(magic_sum_out64, digest, data);
	h.Update(magic_sum_out64, 64);

	h.Final(digest);

	for (int i = 0; i < 998; i++){
		CryptoPP::SHA1 h_cycle;
		h_cycle.Update(digest, 20);
		h_cycle.Final(digest);
	}

	memcpy(key_3des_24 + 20, digest, 4); //add remaining 4 bytes, now we have complete key
	//cout << "DES-EDE24 key "; showarr(key_3des_24, 24, ':'); //DEBUG

	//DES-EDE24
	//CryptoPP::DES_EDE3::Encryption e(key_3des_24, CryptoPP::DES_EDE3::DEFAULT_KEYLENGTH);
	//CryptoPP::ECB_Mode_ExternalCipher::Encryption ecb(e);
	CryptoPP::ECB_Mode<CryptoPP::DES_EDE3>::Encryption ecb(key_3des_24, CryptoPP::DES_EDE3::DEFAULT_KEYLENGTH);
	ecb.ProcessData(response, challenge, 8);
	memcpy(challenge, response, 8);

	delete[] token_pass_u;
}

int Token::et72k_Login(){
	LONG lReturn;
	const DWORD buff_size = 1024;
	DWORD dwRecv;
	int ret = 1;
	BYTE challenge8[8], rte_data20[20], cmd8011t[15];

	if (strlen(this->upass) == 0){
		if (this->debug) cout << "[*] No user password specified, but have to" << endl;
		return 21;
	}

	//if (this->loggedin) return 0;

	BYTE* pbRecv = new BYTE[buff_size];

	lReturn = SCardBeginTransaction(this->hCardHandle);
	if (SCARD_S_SUCCESS != lReturn){
		cout << "[*] ERROR: Failed SCardBeginTransaction: " << lReturn << endl;
		return 20;
	}

	this->sendPreambula();
	dwRecv = buff_size;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, s8, ls8, NULL, pbRecv, &dwRecv); //send s8
	dwRecv = buff_size;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, s9, ls9, NULL, pbRecv, &dwRecv); //send s9
	dwRecv = buff_size;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, s10, ls10, NULL, pbRecv, &dwRecv); //send s10



	//get 20bytes RTE const
	memset(pbRecv, 0, buff_size);
	dwRecv = buff_size;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, cmdGIVE_RTE20, lcmdGIVE_RTE20, NULL, pbRecv, &dwRecv);
	memcpy(rte_data20, pbRecv, 20);
	if (this->debug){ 
		cout << "[*] token RTE data: "; 
		showarr(rte_data20, 20, ':'); }

	//get challenge
	memset(pbRecv, 0, buff_size);
	dwRecv = buff_size;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, cmd8017, lcmd8017, NULL, pbRecv, &dwRecv);
	memcpy(challenge8, pbRecv, 8);
	if (this->debug){ 
		cout << "[*] token challenge: "; 
		showarr(challenge8, 8, ':'); 
	}

	//authenticate
	this->coumpute_authcode(challenge8, rte_data20);
	if (this->debug){
		cout << "[*] app response: ";
		showarr(challenge8, 8, ':');
	}
	memcpy(cmd8011t, cmd8011, lcmd8011);
	memcpy(cmd8011t + 7, challenge8, 8);
	memset(pbRecv, 0, buff_size);
	dwRecv = buff_size;
	SCardTransmit(this->hCardHandle, SCARD_PCI_T1, cmd8011t, lcmd8011, NULL, pbRecv, &dwRecv);
	if (this->debug){
		cout << "[*] et72k_Login finished with: ";
		showarr(pbRecv, 2, ':');
	}
	if (memcmp(pbRecv, resOK, 2) == 0){
		ret = 0;
		//this->loggedin = true;
	}

	//End transaction
	SCardEndTransaction(this->hCardHandle, SCARD_LEAVE_CARD);

	delete[] pbRecv;

	return ret;
}

void Token::invert_bytes(BYTE a[], int n){
	int half = n / 2;

	for (int i = 0; i < half; i++){
		BYTE t = a[i];
		a[i] = a[n - 1 - i];
		a[n - 1 - i] = t;
	}
}

void Token::magic_sum64(BYTE out[64], BYTE v1[20], BYTE v2[64]){
	BYTE v1_64[64];

	memset(out, 0, 64);
	for (int i = 0; i < 64; i++) v1_64[i] = v1[i % 20]; // now v1_64 is 64 bytes

	invert_bytes(v1_64, 64);
	invert_bytes(v2, 64);
	//cout << "magic_sum64 v1="; showarr(v1_64, 64, ':');//DEBUG
	//cout << "magic_sum64 v2="; showarr(v2, 64, ':');//DEBUG

	BYTE carry = 1;

	for (int i = 0; i < 64; i++){
		unsigned int r = v1_64[i] + v2[i] + carry;
		carry = r >> 8;
		out[i] = r & 255;
	}

	invert_bytes(out, 64);
	//cout << "magic_sum64 "; showarr(out, 64, ':');//DEBUG
}

void Token::sendAPDUFromFile(char *f){
	LONG lReturn;
	const DWORD  cdwRecv = 1024;
	DWORD dwRecv;
	BYTE *pbRecv;


	pbRecv = new BYTE[cdwRecv];

	ifstream fin(f);

	if (!fin.is_open()){
		cout << "[*] ERROR: Failed open '" << f <<"'" << endl;
		return;
	}

	string line;
	while (getline(fin, line)){
		if (line.find(">>> ") == 0){
			line.replace(0, 4, "");

			vector<string> sar;
			vector<BYTE> bar;
			split(line, ":", sar);
			hex2byte(sar, bar);

			//
			// Send data to card
			//
			DWORD dwRecv = cdwRecv;
			lReturn = SCardTransmit(hCardHandle,
				SCARD_PCI_T1,
				bar.data(),
				bar.size(),
				NULL,
				pbRecv,
				&dwRecv);
			if (SCARD_S_SUCCESS != lReturn)	{
				cout << "[*] ERROR: Failed SCardTransmit (" << bar << "):" << lReturn << endl;
				goto THE_END;
			}
			if (this->debug){
				cout << ">>> "; showarr(bar.data(), bar.size(), ':');
				cout << "<<< "; showarr(pbRecv, dwRecv, ':');
			}
		}
	}

	THE_END:
	fin.close();

	delete[] pbRecv;

}
