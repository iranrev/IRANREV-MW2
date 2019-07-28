#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string.h>

using namespace System;  
using namespace std;

//Related to MacID
#include <Iphlpapi.h>
#include <Assert.h>
#pragma comment(lib, "iphlpapi.lib")

const char* getUserName()       
{        
	static char UserName[1024]; 
	DWORD size = 1024;     
	GetUserNameA( UserName, &size );           
	return &(UserName[0]);      
}
const char* getMachineName()       
{        
	static char computerName[1024]; 
	DWORD size = 1024;     
	GetComputerNameA( computerName, &size );           
	return &(computerName[0]);      
}

char* MacID = 0;

char* GetMacID(){
	if (MacID) return MacID;

	PIP_ADAPTER_INFO AdapterInfo;
	DWORD dwBufLen = sizeof(AdapterInfo);
	char *mac_addr = (char*)malloc(17);

	AdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));
	if (AdapterInfo == NULL) {
		//printf("Error allocating memory needed to call GetAdaptersinfo\n");

	}

	// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen     variable
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {

		AdapterInfo = (IP_ADAPTER_INFO *) malloc(dwBufLen);
		if (AdapterInfo == NULL) {
			//printf("Error allocating memory needed to call GetAdaptersinfo\n");
		}
	}

	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
		do {
			sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
				pAdapterInfo->Address[0], pAdapterInfo->Address[1],
				pAdapterInfo->Address[2], pAdapterInfo->Address[3],
				pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
			//printf("Address: %s, mac: %s\n", pAdapterInfo->IpAddressList.IpAddress.String, mac_addr);
			MacID = mac_addr;
			return mac_addr;

			//printf("\n");
			pAdapterInfo = pAdapterInfo->Next;        
		}while(pAdapterInfo);                        
	}
	free(AdapterInfo);
}

//#########################################################################
//
//         Some shits Originally Added by Hosseinpourziyaie :)
//                    Actually Imported from Mw3
//
//#########################################################################

// Used Beder Acosta Borges Solution for Spilt string 
// https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c

bool endsWith(const std::string& s, const std::string& suffix)
{
	return s.size() >= suffix.size() &&
		s.substr(s.size() - suffix.size()) == suffix;
}

std::vector<std::string> split(const std::string& s, const std::string& delimiter, const bool& removeEmptyEntries = false)
{
	std::vector<std::string> tokens;

	for (size_t start = 0, end; start < s.length(); start = end + delimiter.length())
	{
		size_t position = s.find(delimiter, start);
		end = position != string::npos ? position : s.length();

		std::string token = s.substr(start, end - start);
		if (!removeEmptyEntries || !token.empty())
		{
			tokens.push_back(token);
		}
	}

	if (!removeEmptyEntries &&
		(s.empty() || endsWith(s, delimiter)))
	{
		tokens.push_back("");
	}

	return tokens;
}
//Solved Above Problem By Doing Function This way
string GetSpiltArrayValue(char *EntryText ,char *SplitterChar ,int Index)
{
	vector <string> ResponseArray;
	ResponseArray = split(EntryText, SplitterChar);
	string str = ResponseArray[Index];
	char *cstr = &str[0u];
	//DisplayResourceNAMessageBox(cstr);
	return str;
}

// How to: Convert System::String to Standard String
// https://msdn.microsoft.com/en-us/library/1b4az623.aspx
  
void MarshalString ( String ^ s, string& os ) {  
   using namespace Runtime::InteropServices;  
   const char* chars =   
      (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();  
   os = chars;  
   Marshal::FreeHGlobal(IntPtr((void*)chars));  
}  
  
void MarshalString ( String ^ s, wstring& os ) {  
   using namespace Runtime::InteropServices;  
   const wchar_t* chars =   
      (const wchar_t*)(Marshal::StringToHGlobalUni(s)).ToPointer();  
   os = chars;  
   Marshal::FreeHGlobal(IntPtr((void*)chars));  
}  