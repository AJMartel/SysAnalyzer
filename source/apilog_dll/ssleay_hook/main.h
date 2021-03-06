/*
'License:   GPL
'Copyright: 2005 iDefense a Verisign Company
'Site:      http://labs.idefense.com
'
'Author:    David Zimmer <david@idefense.com, dzzie@yahoo.com>
'
'         This program is free software; you can redistribute it and/or modify it
'         under the terms of the GNU General Public License as published by the Free
'         Software Foundation; either version 2 of the License, or (at your option)
'         any later version.
'
'         This program is distributed in the hope that it will be useful, but WITHOUT
'         ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
'         FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
'         more details.
'
'         You should have received a copy of the GNU General Public License along with
'         this program; if not, write to the Free Software Foundation, Inc., 59 Temple
'         Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <intrin.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

int  (*Real_SSL_read)(void *s,void *buf,int num) = NULL;
int  (*Real_SSL_write)(void *s,void *buf,int num) = NULL;

void msg(char);
void LogAPI(const char*, ...);

bool Warned=false;
HWND hServer=0;
int DumpAt=0;

extern int myPID;

void GetDllPath(char* buf){ //returns full path of dll
	
	HMODULE h = GetModuleHandleA("api_log.dll");
	if( h == NULL ) h = GetModuleHandleA("api_log.x64.dll");
	if( h == NULL ){ buf[0]=0; return;}

	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0 , GetCurrentProcessId() );
	GetModuleFileNameExA(hProc, h , buf, MAX_PATH);
	CloseHandle(hProc);

}

char* ipfromlng(SOCKADDR_IN* sck){
	
	char *ip = (char*)malloc(16);
	unsigned char *x= (unsigned char*)(((int)sck)+4);
	sprintf(ip,"%d.%d.%d.%d\x00", x[0], x[1], x[2], x[3]);
	return ip;

}




//todo: if we cant find message window dump to log file?

void FindVBWindow(){
	char *vbIDEClassName = "ThunderFormDC" ;
	char *vbEXEClassName = "ThunderRT6FormDC" ;
	char *vbWindowCaption = "ApiLogger" ;

	hServer = FindWindowA( vbIDEClassName, vbWindowCaption );
	if(hServer==0) hServer = FindWindowA( vbEXEClassName, vbWindowCaption );

	if(hServer==0){
		if(!Warned){
			MessageBox(0,"Could not find msg window","",0);
			Warned=true;
		}
	}
	else{
		if(!Warned){
			//first time we are being called we could do stuff here...
			Warned=true;

		}
	}	

} 

char msgbuf[0x1001];

int msg(char *Buffer){
  
  if(hServer==0) FindVBWindow();
  
  COPYDATASTRUCT cpStructData;
  memset(&cpStructData,0, sizeof(struct tagCOPYDATASTRUCT )) ;
  
  _snprintf(msgbuf, 0x1000, "%x,%s", myPID, Buffer);

  cpStructData.dwData = 3;
  cpStructData.cbData = strlen(msgbuf) ;
  cpStructData.lpData = (void*)msgbuf;
  
  int ret = SendMessage(hServer, WM_COPYDATA, 0,(LPARAM)&cpStructData);

  //if ret == 0x then do something special like reconfig ?

  return ret;

} 

void LogAPI(const char *format, ...)
{
	DWORD dwErr = GetLastError();
		
	if(format){
		char buf[1024]; 
		va_list args; 
		va_start(args,format); 
		try{
 			 _vsnprintf(buf,1024,format,args);
			 msg(buf);
		}
		catch(...){}
	}

	SetLastError(dwErr);
}

#define	CalledFrom() (int)_ReturnAddress()

/*__declspec(naked) int CalledFrom(){ 
	
	_asm{
			 mov eax, [ebp+4]  //return address of parent function (were nekkid)
			 ret
	}
	
}*/

 

