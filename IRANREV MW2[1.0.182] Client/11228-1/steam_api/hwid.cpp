#include "stdafx.h"
#include "base64.h"
#include <Iphlpapi.h>
#include <shellapi.h>

unsigned int jenkins_one_at_a_time_hash(char *key, size_t len);

// a funny thing is how this va() function could possibly come from leaked IW code.
#define VA_BUFFER_COUNT		4
#define VA_BUFFER_SIZE		32768 

static char g_vaBuffer[VA_BUFFER_COUNT][VA_BUFFER_SIZE];
static int g_vaNextBufferIndex = 0;
 
const char *va( const char *fmt, ... )
{
	va_list ap;
	char *dest = &g_vaBuffer[g_vaNextBufferIndex][0];
	g_vaNextBufferIndex = (g_vaNextBufferIndex + 1) % VA_BUFFER_COUNT;
	va_start(ap, fmt);
	int res = _vsnprintf( dest, VA_BUFFER_SIZE, fmt, ap );
	dest[VA_BUFFER_SIZE - 1] = '\0';
	va_end(ap);

	if (res < 0 || res >= VA_BUFFER_SIZE)
	{
		//Com_Error(1, "Attempted to overrun string in call to va() - return address 0x%x", _ReturnAddress());
	}

	return dest;
}


char macAddress[15] = "";
BYTE macid[6];

int getMAC(char *macadr)
{
	PIP_ADAPTER_INFO AdapterInfo;
	DWORD dwBufLen = sizeof(AdapterInfo);
	char *mac_addr = (char*)malloc(17);
	
	strcpy(macadr, "");
	AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));

	if (AdapterInfo == NULL) 
	{
		return 0;
	}
	
	// make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) 
	{
		AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);

		if (AdapterInfo == NULL)
		{
			return 0;
		}
	}
	
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) 
	{
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // contains pointer to current adapter info
		
		sprintf(macadr, "%02X%02X%02X%02X%02X%02X",
			pAdapterInfo->Address[0],
			pAdapterInfo->Address[1],
			pAdapterInfo->Address[2],
			pAdapterInfo->Address[3],
			pAdapterInfo->Address[4],
			pAdapterInfo->Address[5]);

		memcpy(&macid[0], &pAdapterInfo->Address[0], 6);
		free(AdapterInfo);

		return 1;
	}

	return 0;
}

unsigned int GetHardwareID()
{
	std::string hwid;
	HW_PROFILE_INFO hwProfileInfo;

	// get MAC address
	getMAC(macAddress);	

	if (GetCurrentHwProfile(&hwProfileInfo) != NULL && getMAC(macAddress) == 1)
	{ // if both are fine
		hwid = va("%s%s", hwProfileInfo.szHwProfileGuid, macAddress);
	}
	else if (GetCurrentHwProfile(&hwProfileInfo) != NULL && getMAC(macAddress) != 1)
	{ // if GetCurrentHwProfile isfine but getMAC isbad
		hwid = va("%s", hwProfileInfo.szHwProfileGuid);
	}
	else if (GetCurrentHwProfile(&hwProfileInfo) == NULL && getMAC(macAddress) == 1)
	{ // if GetCurrentHwProfile is bad but getMAC is fine
		hwid = va("%s", macAddress);
	}
	else
	{
		MessageBoxA(NULL, "Unable to generate hardware ID. Please report this error via email.", "Error", MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), 0);
	}

	// encode with base64
	size_t idLen;
	char* idEncoded = base64_encode((unsigned char*)hwid.c_str(), hwid.size(), &idLen);

	return jenkins_one_at_a_time_hash(idEncoded, strlen(idEncoded));
}
