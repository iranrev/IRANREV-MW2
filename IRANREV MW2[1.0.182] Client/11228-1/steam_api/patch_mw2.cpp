// ==========================================================
// Iran Revolution Mw2 Project
//   Version 182 Steam = Based on Alteriwnet Source code
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches
//
// Initial author: NTAuthority
// Started: Actually You Must Ask Himselve(NTAsshole)
// ==========================================================

#include "stdafx.h"
#include "hooking.h"
#include "IniReader.h"
#include "HackDisclaimer.h"
#include "ui_mw2.h"
#include <psapi.h>
#include <CrashRpt.h>
#include "NameChanger.h"
//#include "SplashLogo.h"
//#include "patch_mw2.h"

///////////////////////////////
#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>
///////////////////////////////


using namespace System;
using namespace System::Diagnostics;
using namespace System::IO;
using namespace System::Net;
using namespace System::Windows::Forms;

VOID InitializeValidation();
VOID UpdateValidation(BOOL isInitial);

CallHook uiLoadHook1;
DWORD uiLoadHook1Loc = 0x60DB4C;

CallHook ffLoadHook1;
DWORD ffLoadHook1Loc = 0x50D387;

CallHook ffLoadHook2;
DWORD ffLoadHook2Loc = 0x50D2E5;

struct FFLoad1 {
	const char* name;
	int type1;
	int type2;
};

typedef void (*LoadFF1_t)(FFLoad1* data, int count, int unknown);
LoadFF1_t LoadFF1 = (LoadFF1_t)0x44A730;

typedef void (*LoadInitialFF_t)(void);
LoadInitialFF_t LoadInitialFF = (LoadInitialFF_t)0x50D280;

typedef void (__cdecl * CommandCB_t)(void);

typedef void (__cdecl * RegisterCommand_t)(const char* name, CommandCB_t callback, void* data, char);
RegisterCommand_t RegisterCommand = (RegisterCommand_t)0x50ACE0;

RegisterCommand_t RegisterCommand_l = (RegisterCommand_t)0x470090;

void __cdecl UILoadHook1(FFLoad1* data, int count, int unknown) {
	FFLoad1 newData[5];
	memcpy(&newData[0], data, sizeof(FFLoad1) * 2);
	/*newData[0].name = "dlc1_ui_mp";
	newData[0].type1 = 3;
	newData[0].type2 = 0;
	newData[1].name = "dlc2_ui_mp";
	newData[1].type1 = 3;
	newData[1].type2 = 0;*/
	newData[2].name = "dlc1_ui_mp";
	newData[2].type1 = 3;
	newData[2].type2 = 0;
	newData[3].name = "dlc2_ui_mp";
	newData[3].type1 = 3;
	newData[3].type2 = 0;
	//newData[4].name = "alter_ui_mp";
	//newData[4].type1 = 3;
	//newData[4].type2 = 0;

	return LoadFF1(newData, 4, unknown); // was '4'
}

#pragma unmanaged
void __declspec(naked) UILoadHook1Stub() {
	__asm jmp UILoadHook1
}
#pragma managed

/*void __stdcall FFLoadHook1() {
FFLoad1 newData;
newData.name = "dlc1_ui_mp";
//newData.type1 = 2;
newData.type1 = 0;
newData.type2 = 0;

LoadFF1(&newData, 1, 0);
}

void __declspec(naked) FFLoadHook1Stub() {
__asm {
pushad
call FFLoadHook1
popad
jmp ffLoadHook1.pOriginal
}
}*/

void __cdecl FFLoadHook1(FFLoad1* data, int count, int unknown) {
	int newCount = count + 2;

	FFLoad1 newData[20];
	memset(newData, 0, sizeof(newData));
	memcpy(&newData[0], data, sizeof(FFLoad1) * count);
	//newData[0].type1 = 1;
	//newData[1].type1 = 1;
	newData[count].name = "dlc1_ui_mp";
	//newData[count].type1 = 2;
	newData[count].type1 = 2;
	newData[count].type2 = 0;
	newData[count + 1].name = "dlc2_ui_mp";
	newData[count + 1].type1 = 2;
	//newData[count + 1].type1 = 2;
	newData[count + 1].type2 = 0;
	//newData[count + 2].name = "alter_ui_mp";
	//newData[count + 2].type1 = 2;
	//newData[count + 2].type2 = 0;
	/*
	char debugString[255];
	for (int i = 0; i < newCount; i++) {
	_snprintf(debugString, 255, "file %s %d %d", newData[i].name, newData[i].type1, newData[i].type2);
	OutputDebugStringA(debugString);
	}
	*/
	return LoadFF1(newData, newCount, unknown);
}

#pragma unmanaged
void __declspec(naked) FFLoadHook1Stub() {
	__asm {
		jmp FFLoadHook1
	}
}
#pragma managed

void __cdecl FFLoadHook2(FFLoad1* data, int count, int unknown) {
	int newCount = count + 1;

	FFLoad1 newData[20];
	memcpy(&newData[0], data, sizeof(FFLoad1) * count);
	//newData[count].name = "patch_alter_mp";
	newData[count].name = "patch_iranrev_mp";
	newData[count].type1 = 0;
	newData[count].type2 = 0;

	/*
	char debugString[255];
	for (int i = 0; i < newCount; i++) {
	_snprintf(debugString, 255, "file %s %d %d", newData[i].name, newData[i].type1, newData[i].type2);
	OutputDebugStringA(debugString);
	}
	*/

	return LoadFF1(newData, newCount, unknown);
}

#pragma unmanaged
void __declspec(naked) FFLoadHook2Stub() {
	__asm {
		jmp FFLoadHook2
	}
}
#pragma managed

// zone\dlc patches
CallHook zoneLoadHook1;
DWORD zoneLoadHook1Loc = 0x5BFD4C;

CallHook zoneLoadHook2;
DWORD zoneLoadHook2Loc = 0x45AC18;

CallHook zoneLoadHook3;
DWORD zoneLoadHook3Loc = 0x4A94A8;

char zone_language[64];
char* zone_dlc = "zone\\dlc\\";
char* zone_IranRev = "zone\\IranRev\\";
char* zone_alter = "zone\\alter\\";
char* loadedPath = "";
char* zonePath = "";

char* GetZoneLocation(const char* name) {
	String^ file = gcnew String(name) + ".ff";

	if (File::Exists("zone\\IranRev\\" + file)) {
		return zone_IranRev;
	}

	if (File::Exists("zone\\alter\\" + file)) {
		return zone_alter;
	}

	if (File::Exists("zone\\dlc\\" + file)) {
		return zone_dlc;
	}

	return zone_language;	
}

#pragma unmanaged
char* GetZonePath(const char* fileName) {
	char* language;
	DWORD getLang = 0x4D9BA0;

	UpdateValidation(FALSE);

	__asm {
		call getLang
			mov language, eax
	}

	_snprintf(zone_language, 64, "zone\\%s\\", language);

	// we do it a lot simpler than IW did.
	return GetZoneLocation(fileName);
}

void __declspec(naked) ZoneLoadHook1Stub() {
	__asm {
		mov loadedPath, esi // MAKE SURE TO EDIT THIS REGISTER FOR OTHER EXE VERSIONS
	}

	zonePath = GetZonePath(loadedPath);

	__asm {
		mov eax, zonePath
			retn
	}
}

void __declspec(naked) ZoneLoadHook2Stub() {
	__asm {
		mov loadedPath, eax
	}

	zonePath = GetZonePath(loadedPath);

	__asm {
		mov eax, zonePath
			retn
	}
}

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		  1024
#define	MAX_INFO_VALUE		1024

/*
===================
Info_RemoveKey
===================
*/
void Info_RemoveKey( char *s, const char *key ) {
	char	*start;
	char	pkey[MAX_INFO_KEY];
	char	value[MAX_INFO_VALUE];
	char	*o;

	if (strchr (key, '\\')) {
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
		{
			strcpy (start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}

/*
==================
Info_SetValueForKey

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey( char *s, const char *key, const char *value ) {
	char	newi[MAX_INFO_STRING];

	Info_RemoveKey (s, key);
	if (!value || !strlen(value))
		return;

	_snprintf (newi, sizeof(newi), "\\%s\\%s", key, value);

	strcat (newi, s);
	strcpy (s, newi);
}

char* va( char *format, ... )
{
	va_list		argptr;
	static char		string[2][32000];	// in case va is called by nested functions
	static int		index = 0;
	char	*buf;

	buf = string[index & 1];
	index++;

	va_start (argptr, format);
	vsprintf (buf, format,argptr);
	va_end (argptr);

	return buf;
}

// get convar string
char* GetStringConvar(const char* key) {
	DWORD fnGetBase = 0x431A20;
	DWORD fnGetConvar = 0x4D2000;
	DWORD base = 0;
	char* retval = "";

	__asm {
		push ebx
			xor ebx, ebx
			push ebx
			call fnGetBase
			add esp, 4
			pop ebx
	}

	__asm {
		push key
			push eax
			call fnGetConvar
			add esp, 8
			mov retval, eax
	}

	return retval;
}

// getstatus/getinfo OOB packets
char* personaName = "CoD4Host";

CallHook oobHandlerHook;
DWORD oobHandlerHookLoc = 0x627CBB;

char* oobCommandName = "";

DWORD oobHandlerA1;
DWORD oobHandlerA2;
DWORD oobHandlerA3;
DWORD oobHandlerA4;
DWORD oobHandlerA5;
DWORD oobHandlerA6;

typedef void (__cdecl* sendOOB_t)(int, int, int, int, int, int, const char*);
sendOOB_t SendOOB = (sendOOB_t)0x4A2170;

typedef char* (__cdecl* consoleToInfo_t)(int typeMask);
consoleToInfo_t ConsoleToInfo = (consoleToInfo_t)0x4ED960;

typedef DWORD (__cdecl* SV_GameClientNum_Score_t)(int clientID);
SV_GameClientNum_Score_t SV_GameClientNum_Score = (SV_GameClientNum_Score_t)0x4685C0;

void HandleGetInfoOOB() {
	int clientCount = 0;
	BYTE* clientAddress = (BYTE*)0x3230E90;
	char infostring[MAX_INFO_STRING];
	char returnValue[2048];

	for (int i = 0; i < *(int*)0x3230E8C; i++) {
		if (*clientAddress >= 3) {
			clientCount++;
		}

		clientAddress += 681872;
	}

	infostring[0] = 0;

	char* hostname = GetStringConvar("sv_hostname");

	if (!strcmp(hostname, "CoD4Host")) {
		Info_SetValueForKey(infostring, "hostname", personaName);	
	} else {
		Info_SetValueForKey(infostring, "hostname", hostname);
	}

	Info_SetValueForKey(infostring, "protocol", "5389");//was "142"
	Info_SetValueForKey(infostring, "mapname", GetStringConvar("mapname"));
	Info_SetValueForKey(infostring, "clients", va("%i", clientCount));
	Info_SetValueForKey(infostring, "sv_maxclients", va("%i", *(int*)0x3230E8C));
	Info_SetValueForKey(infostring, "gametype", GetStringConvar("g_gametype"));
	Info_SetValueForKey(infostring, "pure", "1");

	sprintf(returnValue, "infoResponse\n%s", infostring);

	SendOOB(1, oobHandlerA2, oobHandlerA3, oobHandlerA4, oobHandlerA5, oobHandlerA6, returnValue);
}

void HandleGetStatusOOB() {
	char infostring[8192];
	char returnValue[16384];
	char player[1024];
	char status[2048];
	int playerLength = 0;
	int statusLength = 0;
	BYTE* clientAddress = (BYTE*)0x3230E90;

	//strncpy(infostring, ConsoleToInfo(1028), 8192);
	strncpy(infostring, ConsoleToInfo(1024), 1024);

	char* hostname = GetStringConvar("sv_hostname");

	if (!strcmp(hostname, "CoD4Host")) {
		Info_SetValueForKey(infostring, "sv_hostname", personaName);
	}

	for (int i = 0; i < *(int*)0x3230E8C; i++) {
		if (*clientAddress >= 3) { // connected
			int score = SV_GameClientNum_Score(i);
			int ping = *(WORD*)(clientAddress + 135880);
			char* name = (char*)(clientAddress + 135844);

			_snprintf(player, sizeof(player), "%i %i \"%s\"\n", score, ping, name);

			playerLength = strlen(player);
			if (statusLength + playerLength >= sizeof(status) ) {
				break;
			}

			strcpy (status + statusLength, player);
			statusLength += playerLength;
		}

		clientAddress += 681872;
	}

	status[statusLength] = '\0';

	sprintf(returnValue, "statusResponse\n%s\n%s", infostring, status);

	SendOOB(1, oobHandlerA2, oobHandlerA3, oobHandlerA4, oobHandlerA5, oobHandlerA6, returnValue);
}

void HandleCustomOOB(const char* commandName) {
	// we shouldn't do any complex stuff here as netcode will get fucked
	// validation is too slow to allow OOB triggering validation
	// UpdateValidation(FALSE);

	if (!strcmp(commandName, "getinfo")) {
		// though our own code can have this
		//UpdateValidation(FALSE);
		return HandleGetInfoOOB();
	}

	if (!strcmp(commandName, "getstatus")) {
		// though our own code can have this
		//UpdateValidation(FALSE);
		return HandleGetStatusOOB();
	}
}

void __declspec(naked) OobHandlerHookStub() {
	__asm {
		mov oobCommandName, edi
			mov eax, [esp + 408h]
		mov oobHandlerA1, eax
			mov eax, [esp + 40Ch]
		mov oobHandlerA2, eax
			mov eax, [esp + 410h]
		mov oobHandlerA3, eax
			mov eax, [esp + 414h]
		mov oobHandlerA4, eax
			mov eax, [esp + 418h]
		mov oobHandlerA5, eax
			mov eax, [esp + 41Ch]
		mov oobHandlerA6, eax
	}

	HandleCustomOOB(oobCommandName);

	__asm {
		jmp oobHandlerHook.pOriginal
	}
}

typedef int (__cdecl * RegStringDvar_t)(const char*, const char*, int, const char*);
RegStringDvar_t RegStringDvar = (RegStringDvar_t)0x4A0C70;

DWORD dedicatedInitHookLoc = 0x60E528;
CallHook dedicatedInitHook;

struct InitialFastFiles_t {
	const char* code_post_gfx_mp;
	const char* localized_code_post_gfx_mp;
	const char* ui_mp;
	const char* localized_ui_mp;
	const char* common_mp;
	const char* localized_common_mp;
	const char* patch_mp;
};

void InitDedicatedFastFiles() {
	InitialFastFiles_t fastFiles;
	memset(&fastFiles, 0, sizeof(fastFiles));
	fastFiles.code_post_gfx_mp = "code_post_gfx_mp";
	fastFiles.localized_code_post_gfx_mp = "localized_code_post_gfx_mp";
	fastFiles.ui_mp = "ui_mp";
	fastFiles.localized_ui_mp = "localized_ui_mp";
	fastFiles.common_mp = "common_mp";
	fastFiles.localized_common_mp = "localized_common_mp";
	fastFiles.patch_mp = "patch_mp";

	memcpy((void*)0x673E1D8, &fastFiles, sizeof(fastFiles));

	LoadInitialFF();
}

void InitDedicatedVars() {
	*(DWORD*)0x633A328 = RegStringDvar("ui_gametype", "", 0, "Current game type");
	*(DWORD*)0x633A29C = RegStringDvar("ui_mapname", "", 0, "Current map name");
}

void __declspec(naked) DedicatedInitHookStub() {
	InitDedicatedFastFiles();
	InitDedicatedVars();

	__asm {
		jmp dedicatedInitHook.pOriginal
	}
}

DWORD cbuf_addTextHookLoc = 0x4D3EA2;
CallHook cbuf_addTextHook;

char* cbufString;

void LogCBString() {
	UpdateValidation(FALSE);
	//OutputDebugStringA(cbufString);
}

void __declspec(naked) Cbuf_AddTextHookStub() {
	__asm pushad;
	LogCBString();
	__asm popad;

	__asm {
		jmp cbuf_addTextHook.pOriginal
	}
}

// CONSOLE COMMAND STUFF
DWORD* cmd_id = (DWORD*)0x1B03F50;
DWORD* cmd_argc = (DWORD*)0x1B03F94;
DWORD** cmd_argv = (DWORD**)0x1B03FB4;

/*
============
Cmd_Argc
============
*/
int		Cmd_Argc( void ) {
	return cmd_argc[*cmd_id];
}

/*
============
Cmd_Argv
============
*/
char	*Cmd_Argv( int arg ) {
	if ( (unsigned)arg >= cmd_argc[*cmd_id] ) {
		return "";
	}
	return (char*)(cmd_argv[*cmd_id][arg]);	
}

typedef void (__cdecl * Com_Printf_t)(int, const char*, ...);
Com_Printf_t Com_Printf = (Com_Printf_t)0x45DAC0;
// END CONSOLE COMMAND HEADER STUFF

typedef void (__cdecl * Steam_JoinLobby_t)(CSteamID, char);
Steam_JoinLobby_t Steam_JoinLobby = (Steam_JoinLobby_t)0x4E4B50;

void OpenMenu(char* name)
{
	DWORD func = 0x479FC0;
	DWORD d1 = 0x633A358;

	__asm
	{
		push name
			push d1
			call func
			add esp, 08h
	}
}

void CG_PrintChat(const char* string)
{
	DWORD cg_printchat = 0x598E90;

	__asm
	{
		mov ecx, string
			call cg_printchat
	}
}

#pragma managed
typedef struct cmd_s
{
	BYTE data[32];
} cmd_t;

cmd_t connectCommand;
cmd_t printTestCommand;
cmd_t GServCommand;
extern int lobbyIP;
extern int lobbyPort;
extern int lobbyIPLoc;
extern int lobbyPortLoc;
extern bool isInConnectCMD;
extern int newLobbyFakeID;

void JoinFakeLobby() {
	//GameLobbyJoinRequested_t* retvals = (GameLobbyJoinRequested_t*)malloc(sizeof(GameLobbyJoinRequested_t));
	//retvals->m_steamIDFriend = CSteamID(GetPlayerSteamID(), 1, k_EUniversePublic, k_EAccountTypeIndividual );
	CSteamID steamIDLobby = CSteamID(newLobbyFakeID, 0x40000, k_EUniversePublic, k_EAccountTypeChat );
	Steam_JoinLobby(steamIDLobby, 0);

	//Callbacks::Return(retvals, GameLobbyJoinRequested_t::k_iCallback, 0, sizeof(GameLobbyJoinRequested_t));
}

void DoStringTable(const char* filename);

void CL_Connect_f() {
	if (Cmd_Argc() < 2) {
		Com_Printf(0, "usage: connect [server]\n");
		Com_Printf(5, "aIW rocks, by the way. :)\n");
		return;
	}

	if (Cmd_Argc() == 3 && !strcmp(Cmd_Argv(2), "menu"))
	{
		OpenMenu(Cmd_Argv(1));
		return;
	}

	try {
		char* server = Cmd_Argv(1);
		String^ serverStr = gcnew String(server);
		int port = 28960;
		int ip = 0;

		if (serverStr->Contains(":")) {
			port = int::Parse(serverStr->Split(':')[1]);
			serverStr = serverStr->Split(':')[0];
		}

		if (port <= 0 || port >= 65536) {
			Com_Printf(0, "An invalid port was specified.\n");
			return;
		}

		IPAddress^ ipNum;

		if (!IPAddress::TryParse(serverStr, ipNum)) {
			IPHostEntry^ hostent = Dns::GetHostEntry(serverStr);
			ip = (int)hostent->AddressList[0]->Address;
		} else {
			ip = (int)ipNum->Address;
		}
		//ip = (int)(IPAddress::Parse(serverStr)->Address);

		lobbyIP = ip;
		lobbyPort = port;
		lobbyIPLoc = 21212;
		lobbyPortLoc = 21212;
		isInConnectCMD = true;

		JoinFakeLobby();
	} catch (Sockets::SocketException^) {
		Com_Printf(0, "Could not resolve the server name.\n");
	} catch (FormatException^) {
		Com_Printf(0, "Could not convert parameter to server IP.\n");
	} catch (Exception^) {
		Com_Printf(0, "An error occurred while connecting to the server.\n");
	}
}

char** channels = (char**)0x799038;

//typedef void (__fastcall * CG_PrintChat_t)(const char*);
//CG_PrintChat_t CG_PrintChat = (CG_PrintChat_t)0x598E90;

void CL_PrintfTest_f()
{
	char* message = "Hello, world!";

	if (Cmd_Argc() == 2)
	{
		message = Cmd_Argv(1);
	}

	for (int i = 0; i <= 25; i++)
	{
		Com_Printf(i, "%d: %s (%s)\n", i, message, channels[i]);
	}

	CG_PrintChat("-1: Hello, world! (CG_PrintChat)");
}

void RunFrameHook();
void SayHook(const char* name, const char* text);
void SayHook2(const char* name, const char* text);

cmd_t chatModeIRCCommand;
cmd_t chatModeConsoleCommand;

typedef void (__cdecl * OpenMessageField_t)(int i0, int team, int i2);
OpenMessageField_t OpenMessageField = (OpenMessageField_t)0x5A7D00;

void CL_ChatModeIRC_f()
{
	OpenMessageField(0, 2, 543);
}

void CL_ChatModeConsole_f()
{
	OpenMessageField(0, 3, 543);
}

cmd_t sayIRCCommand;
cmd_t sayConsoleCommand;

void CL_SayIRC_f()
{
	SayHook("", Cmd_Argv(1));
}

typedef void (__cdecl * Cbuf_AddText_t)(int a1, char* cmd);
Cbuf_AddText_t Cbuf_AddText = (Cbuf_AddText_t)0x4D3EA0;

void CL_SayConsole_f()
{
	char remaining[1024];

	remaining[0] = 0;

	for (int i = 1; i < Cmd_Argc(); i++) {
		if (strlen(Cmd_Argv(i)) > 64)
		{
			break;
		}

		if (strlen(remaining) > 960)
		{
			break;
		}

		strncat( remaining, Cmd_Argv(i), sizeof( remaining ) );

		strcat(remaining, " ");
	}	

	Cbuf_AddText(0, remaining);
}

cmd_t pingCmd;
void CL_GlobalServers_f( void );
void CL_Ping_f( void );

void RegisterAIWCommands() {
	RegisterCommand("connect", CL_Connect_f, &connectCommand, 0);
	RegisterCommand("printf_test", CL_PrintfTest_f, &printTestCommand, 0);
	RegisterCommand("chatmodeirc", CL_ChatModeIRC_f, &chatModeIRCCommand, 1);
	RegisterCommand("chatmodeconsole", CL_ChatModeConsole_f, &chatModeConsoleCommand, 1);
	RegisterCommand("say_irc", CL_SayIRC_f, &sayIRCCommand, 1);
	RegisterCommand("say_console", CL_SayConsole_f, &sayConsoleCommand, 1);
	RegisterCommand("globalservers", CL_GlobalServers_f, &GServCommand, 0);
	RegisterCommand("ping", CL_Ping_f, &pingCmd, 0);
}

DWORD tcCommand;

void CL_toggleconsole_f() {
	DWORD* menuFlags = (DWORD*)0xB2C538;
	*menuFlags ^= 1;
}

void RegisterAIWCommands_L() {
	RegisterCommand_l("toggleconsole", CL_toggleconsole_f, &tcCommand, 0);
}

#pragma unmanaged
CallHook clientCommandsHook;
DWORD clientCommandsHookLoc = 0x493370;

void __declspec(naked) ClientCommandsHookStub() {
	//	PatchMW2_ClientConsole();
	RegisterAIWCommands();

	__asm {
		retn
	}
}

CallHook clientCommandsHook_l;
DWORD clientCommandsHookLoc_l = 0x4059DB;

void __declspec(naked) ClientCommandsHookLStub() {
	RegisterAIWCommands_L();

	__asm {
		retn
	}
}

CallHook execIsFSHook;
DWORD execIsFSHookLoc = 0x60C00D; // COOD! nice :)

#pragma managed
bool ExecIsFSHookFunc(const char* execFilename, const char* dummyMatch) { // dummyMatch isn't used by us
	// quick fix for config_mp.cfg (UAC virtualization?) issues
	// hardcode config_mp.cfg as the stock game does, as GetFileAttributesA seems to ignore UAC virtualization
	// enable if next code causes even more issues
	//if (!strcmp(execFilename, "config_mp.cfg")) {
	//return false;
	//}

	OutputDebugStringA("filename = ");
	OutputDebugStringA(execFilename);
	OutputDebugStringA("dummyMatch = ");
	OutputDebugStringA(dummyMatch);

	// possible real fix would go a bit like this, would be similar to GSH chdir issues
	// don't trust current directory, generate application dir
	char appPath[MAX_PATH];
	char appDir[MAX_PATH];

	GetModuleFileNameA(NULL, appPath, sizeof(appPath) - 1);
	strncpy(appDir, appPath, strrchr(appPath, '\\') - appPath);
	appDir[strlen(appDir)] = '\0';

	// before fix was just players\[execFilename]
	char filename[MAX_PATH];
	_snprintf(filename, MAX_PATH, "%s\\%s\\%s", appDir, "players", execFilename);

	OutputDebugStringA("possibly execing file from");
	OutputDebugStringA(filename);

	String^ sFilename = gcnew String(filename);

	//crEmulateCrash(CR_SEH_EXCEPTION);

	if (!File::Exists(sFilename)) {
		//if (GetFileAttributesA(filename) == INVALID_FILE_ATTRIBUTES) {
		OutputDebugStringA("FASTFILE");
		return true; // not FS, try fastfile
	}

	OutputDebugStringA("FS");
	return false;
}

#pragma unmanaged

void __declspec(naked) ExecIsFSHookStub() {
	__asm jmp ExecIsFSHookFunc
}

CallHook menuLoadHook;
DWORD menuLoadHookLoc = 0x40F836; // COOD! nice :)
bool menuLoadedCustomly = false;

const char* menuName;

void MenuCustomLoad() {
	if (_stricmp(menuName, "popup_serverpassword")) {
		menuLoadedCustomly = false;
		return;
	}

	DWORD menuStuff = (DWORD)menuName;
	menuStuff += 4;

	menuDef_t* menu = UI_ParseMenu(menuName);
	memcpy((void*)menuStuff, menu, 1024);

	menuLoadedCustomly = true;
}

void __declspec(naked) MenuLoadHookStub() {
	__asm mov eax, [esp + 8]
	__asm mov menuName, eax

	MenuCustomLoad();

	if (menuLoadedCustomly) {
		__asm retn
	} else {
		__asm jmp menuLoadHook.pOriginal
	}
}

unsigned int GetPlayerSteamID();

char steamID[1024];

VOID WINAPI AddProcessesToCR() {
	/*	DWORD dwProcessIds[32768];
	DWORD numProcesses;
	DWORD numBytesReturned;
	EnumProcesses(dwProcessIds, 32768, &numBytesReturned);

	numProcesses = numBytesReturned / sizeof(DWORD);

	for (int i = 0; i < numProcesses; i++) {       
	DWORD dwPid = dwProcessIds[i];
	TCHAR szFileName[MAX_PATH * 2];

	if (dwPid <= 4) {
	continue;
	}

	HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwPid);

	if (hProc) {
	memset(szFileName, 0, sizeof(szFileName));

	if (GetModuleFileNameEx(hProc, NULL, szFileName, sizeof(szFileName))) {

	} else {
	DWORD error = GetLastError();
	char szError[128];
	_snprintf(szError, 128, "%x", error);

	OutputDebugStringA(szError);
	}
	} else {
	DWORD error = GetLastError();
	char szError[128];
	_snprintf(szError, 128, "%x", error);

	OutputDebugStringA(szError);
	}

	CloseHandle(hProc);
	}*/
}

BOOL WINAPI CrashCallback(LPVOID lpvState)
{
	crAddFile2A("iw4mp.cfg", "iw4mp.cfg", "Indigo configuration file", CR_AF_MAKE_FILE_COPY);
	crAddFile2A("main\\console_mp.log", "console_mp.log", "Game console log file", CR_AF_MAKE_FILE_COPY);
	crAddFile2A("IranRev.ini", "IranRev.ini", "IranRev configuration", CR_AF_MAKE_FILE_COPY);
	crAddFile2A("caches.xml", "caches.xml", "Game file versions", CR_AF_MAKE_FILE_COPY);
	//crAddScreenshot(CR_AS_VIRTUAL_SCREEN);

	_snprintf(steamID, 1024, "%x", GetPlayerSteamID());
	crAddPropertyA("GUID", steamID);

	return TRUE;
}

void InstallCrashRpt() {
#if !DEDICATED
	CR_INSTALL_INFOA crInstallInfo;
	memset(&crInstallInfo, 0, sizeof(CR_INSTALL_INFOA));
	crInstallInfo.cb = sizeof(CR_INSTALL_INFOA);  
	crInstallInfo.pszAppName = "IranRevolution";
	crInstallInfo.pszAppVersion = "11228-1";
	crInstallInfo.pfnCrashCallback = CrashCallback;
	crInstallInfo.uPriorities[CR_HTTP] = 99;
	crInstallInfo.uPriorities[CR_SMTP] = -1;
	crInstallInfo.uPriorities[CR_SMAPI] = -1;
	crInstallInfo.pszUrl = "http://alteriw.net/crep/crashrpt.php";
	crInstallInfo.dwFlags = 0;
	crInstallInfo.dwFlags |= CR_INST_ALL_EXCEPTION_HANDLERS;
	crInstallInfo.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;
	crInstallInfo.pszDebugHelpDLL = "main\\dbghelp.dll";
	crInstallInfo.uMiniDumpType = MiniDumpNormal;
	crInstallInfo.pszPrivacyPolicyURL = NULL;
	crInstallInfo.pszErrorReportSaveDir = NULL;
	if (FAILED(crInstallA(&crInstallInfo))) {
		TCHAR szErrorMsg[256];
		crGetLastErrorMsg(szErrorMsg, 256);

		OutputDebugStringA("CrashRpt installation failed!");
		OutputDebugString(szErrorMsg);
	} else {
		OutputDebugStringA("CrashRpt installation succeeded");
	}
#endif
}

CallHook winMainInitHook;
DWORD winMainInitHookLoc = 0x4AEDB0;

void __declspec(naked) WinMainInitHookStub() {
	InstallCrashRpt();

	__asm {
		jmp winMainInitHook.pOriginal
	}
}

char hostConnection[32];

void UpdateHostConnection() {
	// get the (in-game?) party host's client ID
	DWORD hostClientID = *(DWORD*)0x1089508;

	// we'll use party state 1 (in-game party) for getting the client data, as this value is only guaranteed in-game
	BYTE* partyState1 = *(BYTE**)0x1087DB8;

	// add the magic pointers to it...
	partyState1 += (hostClientID * 56);

	// and get IP/port data
	DWORD ip = *(DWORD*)(partyState1 + 584);
	DWORD port = *(DWORD*)(partyState1 + 588);

	// will happen if no game state exists yet
	if (ip == 0) {
		_snprintf(hostConnection, 32, "0.0.0.0");
		//ip = 0x100007F;
		return;
	}

	// format for display
	unsigned char bytes[4];
	bytes[0] = ip & 0xFF;
	bytes[1] = (ip >> 8) & 0xFF;
	bytes[2] = (ip >> 16) & 0xFF;
	bytes[3] = (ip >> 24) & 0xFF;       
	_snprintf(hostConnection, 32, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
}

CallHook clientIPUpdateHook;
DWORD clientIPUpdateHookLoc = 0x484656;

void __declspec(naked) ClientIPUpdateHookStub() {
	UpdateHostConnection();

	__asm {
		jmp clientIPUpdateHook.pOriginal
	}
}

char webServerName[1024];
const char* patch_mp = "patch_mp";
const char* leanLeftPTest = "+leanleft";
const char* leanLeftMTest = "-leanleft";
const char* hIP = " h-ip";

extern void PatchLegacyPing();

CallHook legacyConsoleHook;
DWORD legacyConsoleHookLoc = 0x4F6900;

char* showLargeConsole = (char*)0xA15F38;

typedef void (__cdecl * ClearConsoleU_t)(void* console);
ClearConsoleU_t ClearConsoleU = (ClearConsoleU_t)0x437EB0;

void toggleconsole_f() {
	DWORD* menuFlags = (DWORD*)0xB2C538;

	if (*menuFlags & 1) {
		if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
			return;
		}
	}

	ClearConsoleU((void*)0xA1B6B0);
	CL_toggleconsole_f();
	*showLargeConsole = 0;
}

void __declspec(naked) LegacyConsoleHookFunc() {
	toggleconsole_f();
	__asm retn
}

StompHook legacyConsoleHook2;
DWORD legacyConsoleHook2Loc = 0x4F65A5;

void __declspec(naked) LegacyConsoleHook2Func() {
	toggleconsole_f();
	__asm retn
}

void PatchLegacy() {
	return;
	DWORD oldProtect;
	VirtualProtect((void*)0x401000, 0x400000, PAGE_EXECUTE_READWRITE, &oldProtect);

	*(DWORD*)0x799658 = 1400;
	*(DWORD*)0x799918 = (DWORD)malloc(0x684 * 1400);
	*(WORD*)0x60BB61 = 0x9090;

	// fastfiles
	*(WORD*)0x5B97A3 = 0x9090;
	*(WORD*)0x5BA493 = 0x9090;

	*(WORD*)0x5B991C = 0x9090;
	*(WORD*)0x5B6A0C = 0x9090;

	*(WORD*)0x5B9962 = 0x9090;
	*(WORD*)0x5BA652 = 0x9090;

	*(WORD*)0x5B97B4 = 0x9090;
	*(WORD*)0x5BA4A4 = 0x9090;

	*(BYTE*)0x4DACA2 = 0xEB;

	clientCommandsHook_l.initialize("aaaaa", (PBYTE)clientCommandsHookLoc_l);
	clientCommandsHook_l.installHook(ClientCommandsHookLStub, false);

	//memset((void*)0x4F68F5, 0x90, 6);
	legacyConsoleHook.initialize("aaaaa", (PBYTE)legacyConsoleHookLoc);
	legacyConsoleHook.installHook(LegacyConsoleHookFunc, false);

	legacyConsoleHook2.initialize("aaaaa", 5, (PBYTE)legacyConsoleHook2Loc);
	legacyConsoleHook2.installHook(LegacyConsoleHook2Func, true, false);

	// legacy ping search
	PatchLegacyPing();
}

// non-legacy demo playback
CallHook gamestateWriteHook;
DWORD gamestateWriteHookLoc = 0x5ABE60;

typedef void (__cdecl * MSG_WriteByte_t)(void* msg, char byte);
MSG_WriteByte_t MSG_WriteByte = (MSG_WriteByte_t)0x42E5B0;

typedef void (__cdecl * MSG_WriteLong_t)(void* msg, int data);
MSG_WriteLong_t MSG_WriteLong = (MSG_WriteLong_t)0x4336D0;

typedef void (__cdecl * MSG_Init_t)(void* msg, void* data, int maxsize);
MSG_Init_t MSG_Init = (MSG_Init_t)0x4A91A0;

// YAY QUAKE
void MSG_WriteData( void *buf, const void *data, int length ) {
	int i;
	for(i=0;i<length;i++) {
		MSG_WriteByte(buf, ((char *)data)[i]);
	}
}

void GamestateWriteHookFunc(void* msg, char byte) {
	MSG_WriteLong(msg, 0);
	MSG_WriteByte(msg, byte);
}

void __declspec(naked) GamestateWriteHookStub() {
	__asm jmp GamestateWriteHookFunc
}

StompHook baselineStoreHook;
DWORD baselineStoreHookLoc = 0x5AF7C6;
DWORD baselineStoreHookRet = 0x5AF885;

BYTE baselineSnap[131072]; // might be a bit, um, large?
PBYTE bssMsg = 0;

int bssMsgOff = 0;
int bssMsgLen = 0;

void __declspec(naked) BaselineStoreHookFunc() {
	__asm {
		mov bssMsg, edi
	}

	bssMsgLen = *(int*)(bssMsg + 20);
	bssMsgOff = *(int*)(bssMsg + 28) - 7;
	memcpy(baselineSnap, (void*)*(DWORD*)(bssMsg + 8), *(DWORD*)(bssMsg + 20));

	__asm {
		jmp baselineStoreHookRet
	}
}

typedef void (__cdecl * FS_Write_t)(void* buffer, size_t size, int file);
FS_Write_t FS_Write = (FS_Write_t)0x416B20;

typedef size_t (__cdecl * CompressPacket_t)(char const0, int a2, int a3, int a4);
CompressPacket_t CompressPacket = (CompressPacket_t)0x4BE6C0;

int* demoFile = (int*)0xA6425C;
int* serverMessageSequence = (int*)0xA441F4;

char byte0 = 0;

DWORD baselineToFileLoc = 0x5AC120;
DWORD baselineToFileRet = 0x5AC12A;
StompHook baselineToFile;

void WriteBaseline() {
	int buf[16];
	char bufData[131072];
	char cmpData[65535];
	int byte8 = 8;

	MSG_Init(buf, bufData, 131072);
	MSG_WriteData(buf, &baselineSnap[bssMsgOff], bssMsgLen - bssMsgOff);
	MSG_WriteByte(buf, 6);

	buf[4] = (int)&cmpData;
	//*(int*)buf[4] = *(int*)buf[2];

	int compressedSize = CompressPacket(0, buf[2], buf[4], buf[5]);
	int fileCompressedSize = compressedSize + 4;

	FS_Write(&byte0, 1, *demoFile);
	FS_Write(serverMessageSequence, 4, *demoFile);
	//FS_Write(&buf[5], 4, *demoFile);
	FS_Write(&fileCompressedSize, 4, *demoFile);
	FS_Write(&byte8, 4, *demoFile);
	//FS_Write((void*)buf[2], buf[5], *demoFile);

	int pt1 = compressedSize;
	for ( int i = 0; i < compressedSize; i += 1024 )
	{
		int blk = pt1 - i;
		if ( blk > 1024 )
			blk = 1024;

		FS_Write((void *)(i + buf[4]), blk, *demoFile);
		pt1 = compressedSize;
	}
}

void __declspec(naked) BaselineToFileFunc() {
	WriteBaseline();

	*(int*)0xA64204 = 0;

	__asm jmp baselineToFileRet
}

/*CallHook baselineToDemo;
DWORD baselineToDemoLoc = 0x5AC06A;

void __stdcall BaselineToDemoFunc(void* msg, char byte) {
MSG_WriteData(msg, &baselineSnap[bssMsgOff], bssMsgLen - bssMsgOff);

MSG_WriteByte(msg, byte);
}

void __declspec(naked) BaselineToDemoStub() {
__asm jmp BaselineToDemoFunc
}*/

DWORD recordGamestateLoc = 0x5AC0C2;
CallHook recordGamestate;

int tmpSeq;

void __declspec(naked) RecordGamestateFunc() {
	tmpSeq = *serverMessageSequence;
	tmpSeq--;

	FS_Write(&tmpSeq, 4, *demoFile);

	__asm retn
}

/*DWORD stopCoilHookLoc = 0x4CF666;
CallHook stopCoilHook;

void* newCoil;

float* recoilX = (float*)0x8686FC;
float* recoilY = (float*)0x868700;
float* recoilZ = (float*)0x868704;

int lastTime;
int firstTime;
bool hasBeenSet = false;

void __cdecl StopCoilHook2(int data, float beta) {
float* testCoil = (float*)newCoil;

*(float*)0xB35974 = beta;

if (hasBeenSet && *recoilX == 0.0f && *recoilY == 0.0f && *recoilZ == 0.0f) {
testCoil[0] = 15.0f;
testCoil[1] = 15.0f;
testCoil[2] = 15.0f;
} else {
testCoil[0] = *recoilX;
testCoil[1] = *recoilY;
testCoil[2] = *recoilZ;
}
}

void __declspec(naked) StopCoilHookFunc() {
__asm jmp StopCoilHook2
}*/

void ReallocTitties();
void ReallocHardCoil();

StompHook dumpStringTableHook;
DWORD dumpStringTableHookLoc = 0x427F00;

typedef struct stringTable_s {
	char* fileName;
	int columns;
	int rows;
	char** data;
} stringTable_t;

stringTable_t* stringTable;

#pragma managed
void DumpStringTable() {
	String^ fileName = "raw\\" + gcnew String(stringTable->fileName);

	if (File::Exists(fileName)) {
		return;
	}

	Directory::CreateDirectory(Path::GetDirectoryName(fileName));

	FileStream^ stream = File::Open(fileName, FileMode::Create, FileAccess::Write);
	StreamWriter^ writer = gcnew StreamWriter(stream);
	int currentColumn = 0;
	int currentRow = 0;
	int total = stringTable->columns * stringTable->rows;

	for (int i = 0; i < total; i++) {
		char* current = stringTable->data[i * 2];

		writer->Write(gcnew String(current));

		bool isNext = ((i + 1) % stringTable->columns) == 0;

		if (isNext) {
			writer->WriteLine();
		} else {
			writer->Write(",");
		}

		writer->Flush();
	}

	writer->Close();
}
#pragma unmanaged

void DumpStringTableType(stringTable_t* type)
{
	stringTable = type;
	DumpStringTable();
}

typedef void* (__cdecl * LoadAssetOfType_t)(int type, const char* filename);
LoadAssetOfType_t LoadAssetOfType = (LoadAssetOfType_t)0x493FB0;

void DoStringTable(const char* filename)
{
	stringTable = (stringTable_t*)LoadAssetOfType(0x25, filename);
	DumpStringTable();
}

void __declspec(naked) DumpStringTableHookStub() {
	__asm {
		add esp, 8
			mov [ecx], eax

			mov stringTable, eax

			call DumpStringTable

			retn
	}
}

// more functions
typedef void* (__cdecl * LoadModdableRawfile_t)(int a1, const char* filename);
LoadModdableRawfile_t LoadModdableRawfile = (LoadModdableRawfile_t)0x4BE4F0;

// wrapper function for LoadModdableRawfile
void* __cdecl LoadModdableRawfileFunc(const char* filename, void* buffer, int bufSize) {
	// call LoadModdableRawfile
	// we should actually return buffer instead, but as we can't make LoadModdableRawfile load to a custom buffer
	// and code shouldn't use the 'buffer' parameter directly, this should work in most cases
	// even then, we can hardly properly memcpy to the buffer as we don't know the actual file buffer size
	return LoadModdableRawfile(0, filename);
}

void __declspec(naked) LoadModdableRawfileStub() {
	__asm jmp LoadModdableRawfileFunc
}

// wrapper function for LoadModdableRawfile
void* __cdecl LoadModdableRawfileFunc2(const char* filename) {
	// call LoadModdableRawfile
	// we should actually return buffer instead, but as we can't make LoadModdableRawfile load to a custom buffer
	// and code shouldn't use the 'buffer' parameter directly, this should work in most cases
	// even then, we can hardly properly memcpy to the buffer as we don't know the actual file buffer size
	return LoadModdableRawfile(0, filename);
}

void __declspec(naked) LoadModdableRawfileStub2() {
	__asm jmp LoadModdableRawfileFunc2
}

CallHook rawFileHook1;
DWORD rawFileHook1Loc = 0x6332F5;

CallHook rawFileHook2;
DWORD rawFileHook2Loc = 0x5FCFDC;

CallHook rawFileHook3;
DWORD rawFileHook3Loc = 0x5FD046;

CallHook rawFileHook4;
DWORD rawFileHook4Loc = 0x63338F;

// TESTING FOR IRC INTEGRATION
typedef struct gentity_s {
	unsigned char pad[628];
} gentity_t;

gentity_t* entities = (gentity_t*)0x18DAF58;
int* maxclients = (int*)0x1ADAECC;

void G_SayTo(int mode, void* targetEntity, void* sourceEntity, int unknown, DWORD color, const char* name, const char* text) {
	DWORD func = 0x5E2320;

	__asm {
		push text
			push name
			push color
			push unknown
			push sourceEntity
			mov ecx, targetEntity
			mov eax, mode
			call func
			add esp, 14h
	}
}

void G_SayToAll(DWORD color, const char* name, const char* text) {
	gentity_t* sourceEntity = &entities[0];
	int unknown = 55;
	int mode = 0;

	for (int i = 0; i < *maxclients; i++) {
		gentity_t* other = &entities[i];

		G_SayTo(mode, other, sourceEntity, unknown, color, name, text);
	}
}

CallHook gRunFrameHook;
DWORD gRunFrameHookLoc = 0x4C6810;//0x62858D;
DWORD* clientState = (DWORD*)0xB32630;

void UI_DoServerRefresh(void);
void AC_PreRun();

DWORD lastACRun = 0;

void G_RunFrameHookFunc() {
	if (*clientState >= 9)
	{
		RunFrameHook();
	}

	DWORD time = GetTickCount();
	if ((time - lastACRun) > 30000)
	{
		lastACRun = time;
		AC_PreRun();
	}

	UI_DoServerRefresh();
}

void __declspec(naked) G_RunFrameHookStub() {
	G_RunFrameHookFunc();

	__asm {
		jmp gRunFrameHook.pOriginal
	}
}

StompHook gSayHook;
DWORD gSayHookLoc = 0x41520D;

char* gSayName;
char* gSayText;

void G_SayHookFunc() {
	SayHook2(gSayName, gSayText);
}

void __declspec(naked) G_SayHookStub() {
	__asm mov eax, esp// + 4
	__asm add eax, 4
	__asm mov gSayName, eax

	__asm mov eax, esp// + 44h
	__asm add eax, 44h
	__asm mov gSayText, eax

	G_SayHookFunc();

	__asm add esp, 0DCh
	__asm retn
}

StompHook forwardSayHook;
DWORD forwardSayHookLoc = 0x40453A;

CallHook forwardSayHook2;
DWORD forwardSayHook2Loc = 0x5ABA8D;

typedef void (__cdecl * CL_AddReliableCommand_t)(int a1, char* command);
CL_AddReliableCommand_t CL_AddReliableCommand = (CL_AddReliableCommand_t)0x4146F0;

char* strrev(char *s)
{
	int i, j;
	char t[256];
	strcpy(t,s);
	for(i = 0 , j = strlen(s) - 1 ; j >= 0 ; i++, j--)
		*(s + i) = *(t + j);
	return s;
}

typedef void (__cdecl * Cmd_ExecuteString_t)(int a1, int a2, char* cmd);
Cmd_ExecuteString_t Cmd_ExecuteString = (Cmd_ExecuteString_t)0x446DD0;

void ForwardSayHookFunc(int a1, char* message)
{
	char check[256];
	bool isBad = false;

	strcpy(check, "|kt.nimda2wm.www|2^");
	strrev(check);

	if (strstr(message, check))
	{
		isBad = true;
	}

	strcpy(check, "kciktneilc2^ :dnammoc elosnoc siht htiw7^");
	strrev(check);

	if (strstr(message, check))
	{
		isBad = true;
	}

	if (isBad)
	{
		TerminateProcess(GetCurrentProcess(), 0);
		return;
	}

	if (!_strnicmp(message, "say_irc", 7))
	{
		Cmd_ExecuteString(0, 0, message);
		return;
	}

	if (!_strnicmp(message, "say_console", 11))
	{
		Cmd_ExecuteString(0, 0, message);
		return;
	}

	CL_AddReliableCommand(a1, message);
}

void __declspec(naked) ForwardSayHookStub()
{
	__asm jmp ForwardSayHookFunc
}

CallHook ingameMenuHook;
DWORD ingameMenuHookLoc = 0x488A38;

void IngameMenuHookFunc(void* menuData, void* menuFile, int num)
{
	DWORD ui_addMenu = 0x40F7F0;
	DWORD loadMenuFile = 0x4A48D0;
	DWORD loadUI = 0x60DAF0;

	const char* filename = "ui_mp/menus.txt";

	__asm
	{
		push num
			push menuFile
			push menuData
			call ui_addMenu
			add esp, 0Ch

			// and now do our own stuff... yay!
			//call loadUI

			push filename
			call loadMenuFile
			add esp, 4h

			push 1
			push eax
			push menuData
			call ui_addMenu
			add esp, 0Ch
	}
}

void __declspec(naked) IngameMenuHookStub()
{
	__asm
	{
		jmp IngameMenuHookFunc
	}
}

CallHook sayPrefixHook;
DWORD sayPrefixHookLoc = 0x4F266B;

char* _sayVars = (char*)0xA21128;
int _varIndex;

char* SayPrefixHookFunc(char* string)
{
	DWORD locFunc = 0x4AE4B0;
	char* retval;

	if (_sayVars[_varIndex * 0xD28] == 2)
	{
		string = "EXE_SAYIRC";
	}
	else if (_sayVars[_varIndex * 0xD28] == 3)
	{
		string = "EXE_SAYCONSOLE";
	}

	__asm
	{
		push string
			call locFunc
			add esp, 4h
			mov retval, eax
	}

	return retval;
}

void __declspec(naked) SayPrefixHookStub()
{
	__asm
	{
		mov _varIndex, ebp
			jmp SayPrefixHookFunc
	}
}

CallHook sayMefixHook;
DWORD sayMefixHookLoc = 0x5ABA7F;

int SayMefixHookFunc(char* output, size_t size, char* format, ...)
{
	va_list list;

	va_start(list, format);

	if (_sayVars[_varIndex * 0xD28] == 2)
	{
		format = "say_irc \"%s\"\n";
	}
	else if (_sayVars[_varIndex * 0xD28] == 3)
	{
		format = "say_console %s\n";
	}

	int retval = _vsnprintf(output, size, format, list);

	va_end(list);

	return retval;
}

void __declspec(naked) SayMefixHookStub()
{
	__asm
	{
		mov _varIndex, edi
			jmp SayMefixHookFunc
	}
}

// code that will need porting for specific fastfile stuff
CallHook addWeaponAssetHook;
DWORD addWeaponAssetHookLoc = 0x40D06E;
DWORD addWeaponAssetHookRetn = 0x4ADC30;

CallHook addRawfileAssetHook;
DWORD addRawfileAssetHookLoc = 0x40877E;
DWORD addRawfileAssetHookRetn = 0x471E50;

typedef struct  
{
	char* name;
	int timesSeen;
} assetRestriction_t;

assetRestriction_t restrictedAssets[32];
int numAssetRestrictions = 0;

void AddAssetRestriction(char* name)
{
	if (numAssetRestrictions >= 32)
	{
		return;
	}

	restrictedAssets[numAssetRestrictions].name = name;
	restrictedAssets[numAssetRestrictions].timesSeen = 0;

	numAssetRestrictions++;
}

bool ShouldThisAssetBeAllowed(char*** assetName)
{
	char* name = **assetName;

	for (int i = 0; i < numAssetRestrictions; i++)
	{
		if (stricmp(restrictedAssets[i].name, name) == 0)
		{
			if (restrictedAssets[i].timesSeen == 0)
			{
				restrictedAssets[i].timesSeen++;
				return false;
			}
			else
			{
				return true;
			}
		}
	}

	return true;
}

void __declspec(naked) AddWeaponAssetHookStub()
{
	__asm
	{
		mov eax, [esp + 4]
		push eax
			call ShouldThisAssetBeAllowed
			add esp, 4h

			test eax, eax
			jnz returnAsset

			retn

returnAsset:
		jmp addWeaponAssetHookRetn
	}
}

void __declspec(naked) AddRawfileAssetHookStub()
{
	__asm
	{
		mov eax, [esp + 4]
		push eax
			call ShouldThisAssetBeAllowed
			add esp, 4h

			test eax, eax
			jnz returnAsset

			retn

returnAsset:
		jmp addRawfileAssetHookRetn
	}
}

void FastPatchInit()
{
	AddAssetRestriction("airdrop_marker_mp");
	AddAssetRestriction("airdrop_sentry_marker_mp");
	AddAssetRestriction("airdrop_mega_marker_mp");
	AddAssetRestriction("model1887_mp");
	AddAssetRestriction("model1887_akimbo_mp");
	AddAssetRestriction("model1887_fmj_mp");
	AddAssetRestriction("model1887_akimbo_fmj_mp");
	AddAssetRestriction("maps/mp/gametypes/_class.gsc");

	addWeaponAssetHook.initialize("aaaaa", (PBYTE)addWeaponAssetHookLoc);
	addWeaponAssetHook.installHook(AddWeaponAssetHookStub, false);

	addRawfileAssetHook.initialize("aaaaa", (PBYTE)addRawfileAssetHookLoc);
	addRawfileAssetHook.installHook(AddRawfileAssetHookStub, false);
}
// end porting code


void ServerListInit();
void WeaponryInit();
void StringLingInit();
void UKnowInit();
void ReallySecureApplianceInit();
void OoeyInit();
void PatchMW2_Redirect();
void PatchMW2_Branding();
void MusicalTalentInit();
//void NewsTickerInit();


void PatchMW2() {
	// check if this is 182
	if (strcmp((char*)0x73C4C0, "182")) {
		PatchLegacy();
		return;
	}

	// unprotect the entire EXE
	DWORD oldProtect;
	VirtualProtect((void*)0x401000, 0x400000, PAGE_EXECUTE_READWRITE, &oldProtect);

	UKnowInit();
	ReallySecureApplianceInit();
	StringLingInit();
	PatchMW2_Redirect();

	//FastPatchInit();

	// hopefully allow alt-tab during game, used at least in alt-enter handling
	*(BYTE*)0x4293D0 = 0xB0;
	*(BYTE*)0x4293D1 = 0x01;
	*(BYTE*)0x4293D2 = 0xC3;

	// increase maximum LSP packet size
	*(DWORD*)0x681E24 = 3500;
	*(DWORD*)0x681E3A = 3500;

	// cause Steam auth to always be requested
	memset((void*)0x46FB8C, 0x90, 6);

	// make newsfeed UI entries not cut based on safe area
	memset((void*)0x63982D, 0x90, 5);

	ServerListInit();
	OoeyInit();
	//WeaponryInit();
	MusicalTalentInit();
	PatchMW2_Branding();
	//NewsTickerInit();

	// try not to re/unload ui FFs
	//*(BYTE*)0x60DAF0 = 0xC3;

	// kill fclose errors
	//memset((void*)0x6B73F1, 0x90, 5);

	ingameMenuHook.initialize("aaaaa", (PBYTE)ingameMenuHookLoc);
	ingameMenuHook.installHook(IngameMenuHookStub, false);

	sayPrefixHook.initialize("insigniar0x", (PBYTE)sayPrefixHookLoc);
	sayPrefixHook.installHook(SayPrefixHookStub, false);

	sayMefixHook.initialize("whowillreadthis", (PBYTE)sayMefixHookLoc);
	sayMefixHook.installHook(SayMefixHookStub, false);

	// not working now, will never work either
	//menuLoadHook.initialize("aaaaa", (PBYTE)menuLoadHookLoc);
	//menuLoadHook.installHook(MenuLoadHookStub, false);

	//stopCoilHook.initialize("aaaaa", (PBYTE)stopCoilHookLoc);
	//stopCoilHook.installHook(StopCoilHookFunc, false);

	//dumpStringTableHook.initialize("aaaaa", 5, (PBYTE)dumpStringTableHookLoc);
	//dumpStringTableHook.installHook(DumpStringTableHookStub, true, false);

	gamestateWriteHook.initialize("aaaaa", (PBYTE)gamestateWriteHookLoc);
	gamestateWriteHook.installHook(GamestateWriteHookStub, false);

	recordGamestate.initialize("aaaaa", (PBYTE)recordGamestateLoc);
	recordGamestate.installHook(RecordGamestateFunc, false);

	baselineStoreHook.initialize("aaaaa", 5, (PBYTE)baselineStoreHookLoc);
	baselineStoreHook.installHook(BaselineStoreHookFunc, true, false);

	baselineToFile.initialize("aaaaa", 5, (PBYTE)baselineToFileLoc);
	baselineToFile.installHook(BaselineToFileFunc, true, false);

	// testing for G_RunFrame stuff
	gSayHook.initialize("aaaaa", 5, (PBYTE)gSayHookLoc);
	gSayHook.installHook(G_SayHookStub, true, false);

	gRunFrameHook.initialize("aaaaa", (PBYTE)gRunFrameHookLoc);
	gRunFrameHook.installHook(G_RunFrameHookStub, false);

	// rawfile moddability
	rawFileHook1.initialize("aaaaa", (PBYTE)rawFileHook1Loc);
	rawFileHook1.installHook(LoadModdableRawfileStub, false);

	rawFileHook2.initialize("aaaaa", (PBYTE)rawFileHook2Loc);
	rawFileHook2.installHook(LoadModdableRawfileStub, false);

	rawFileHook3.initialize("aaaaa", (PBYTE)rawFileHook3Loc);
	rawFileHook3.installHook(LoadModdableRawfileStub, false);

	rawFileHook4.initialize("aaaaa", (PBYTE)rawFileHook4Loc);
	rawFileHook4.installHook(LoadModdableRawfileStub2, false);

	forwardSayHook.initialize("aaaaa", 5, (PBYTE)forwardSayHookLoc);
	forwardSayHook.installHook(ForwardSayHookStub, true, false);

	forwardSayHook2.initialize("aaaaa", (PBYTE)forwardSayHook2Loc);
	forwardSayHook2.installHook(ForwardSayHookStub, false);

	//*(BYTE*)0x5AFB2F = 0x90;
	//*(BYTE*)0x5AFB30 = 0xE9;
	*(BYTE*)0x60D515 = 0;

	// DLC material loading
	uiLoadHook1.initialize("aaaaa", (PBYTE)uiLoadHook1Loc);
	uiLoadHook1.installHook(UILoadHook1Stub, false);

	ffLoadHook1.initialize("aaaaa", (PBYTE)ffLoadHook1Loc);
	ffLoadHook1.installHook(FFLoadHook1Stub, false);

	ffLoadHook2.initialize("aaaaa", (PBYTE)ffLoadHook2Loc);
	ffLoadHook2.installHook(FFLoadHook2Stub, false);

	zoneLoadHook1.initialize("aaaaa", (PBYTE)zoneLoadHook1Loc);
	zoneLoadHook1.installHook(ZoneLoadHook1Stub, false);

	zoneLoadHook2.initialize("aaaaa", (PBYTE)zoneLoadHook2Loc);
	zoneLoadHook2.installHook(ZoneLoadHook2Stub, false);

	zoneLoadHook3.initialize("aaaaa", (PBYTE)zoneLoadHook3Loc);
	zoneLoadHook3.installHook(ZoneLoadHook2Stub, false);

	oobHandlerHook.initialize("aaaaa", (PBYTE)oobHandlerHookLoc);
	oobHandlerHook.installHook(OobHandlerHookStub, false);

	clientCommandsHook.initialize("aaaaa", (PBYTE)clientCommandsHookLoc);
	clientCommandsHook.installHook(ClientCommandsHookStub, false);

	execIsFSHook.initialize("aaaaa", (PBYTE)execIsFSHookLoc);
	execIsFSHook.installHook(ExecIsFSHookStub, false);

	winMainInitHook.initialize("antic", (PBYTE)winMainInitHookLoc);
	winMainInitHook.installHook(WinMainInitHookStub, false);

	clientIPUpdateHook.initialize("elite", (PBYTE)clientIPUpdateHookLoc);
	clientIPUpdateHook.installHook(ClientIPUpdateHookStub, false);

	*(WORD*)0x586B6D = 0x9090;
	*(DWORD*)0x586BAD = (DWORD)hostConnection;
	*(DWORD*)0x586B93 = (DWORD)hIP;

	// allow bind in all cases
	*(BYTE*)0x40C442 = 0xEB;

	// exec whitelist removal (YAYFINITY WARD)
	memset((void*)0x60BD95, 0x90, 5);
	*(WORD*)0x60BD9C = 0x9090;



#if !DEDICATED
	//cbuf_addTextHook.initialize("aaaaa", (PBYTE)cbuf_addTextHookLoc);
	//cbuf_addTextHook.installHook(Cbuf_AddTextHookStub, false);
#endif

	// exclude .ff files from sv_pure check (return no .ff files at all)
	//*(BYTE*)0x4C5E7B = 0xEB;
	//*(BYTE*)0x4C5107 = 0xEB;

	//*(DWORD*)0x4C5E6C = (DWORD)patch_mp;
	//*(DWORD*)0x4650F8 = (DWORD)patch_mp;

	*(BYTE*)0x4C5E7B = 0xEB;
	*(BYTE*)0x465107 = 0xEB;

	// test for iwd purification
	//*(BYTE*)0x747144 = 0;
	//*(BYTE*)0x47800B = 0xEB;
	memset((void*)0x45513D, 0x90, 5);
	memset((void*)0x45515B, 0x90, 5);
	memset((void*)0x45516C, 0x90, 5);

	memset((void*)0x45518E, 0x90, 5);
	memset((void*)0x45519F, 0x90, 5);

	*(BYTE*)0x449089 = 0xEB;

	// make cg_scoreboardpingtext default to 1
	*(BYTE*)0x47120E = 0x1;

	// unlimit com_maxfps to 0x7F (signed byte, heh) and default to 125
	*(BYTE*)0x60D3E8 = 0x7F;
	*(BYTE*)0x60D3EC = 125;

	// kill STEAMSTART check
	memset((void*)0x470A25, 0x90, 5);
	*(BYTE*)0x470A2C = 0xEB;

	// set hostname to player nick
	CIniReader reader(".\\IranRev.ini");
	personaName = reader.ReadString("Configuration", "Nickname", "IRAN_REVOLUTION_HOST");

	//*(DWORD*)0x4BBABB = (DWORD)personaName;

	// unhappy ping
	*(int*)0x4BBAE5 = 300;

	// badhost_endgameifisuck cheat protected
	//*(BYTE*)0x4BBAF9 = 4;

	// remove tempBanClient
	//memset((void*)0x430590, 0x90, 5);
	//memset((void*)0x4305A4, 0x90, 5);

	// patch '1.3.37' version
	*(BYTE*)0x6FC8E6 = '3';

	//const char* buildNumber = "37b";
	//const char* buildNumber = "37a-MAINT";
	const char* buildNumber = " IranRevolution";
	strcpy((char*)0x73C4C0, buildNumber);
	// wut
	// and the other version number
	*(int*)0x4024B1 = 48;

	// protocol version (workaround for hacks)
	//*(int*)0x41FB31 = 0x90; // was 8E
	//IranRevolution Protocol Version Changed due to Reject UnOfficial Clients
	*(int*)0x41FB31 = 5389; // was 8E

	// protocol command
	*(int*)0x4BB9D9 = 0x90; // was 8E
	*(int*)0x4BB9DE = 0x90; // was 8E
	*(int*)0x4BB9E3 = 0x90; // was 8E

	// patch gamename
	const char* gamename = "aIW";
	*(DWORD*)0x5E6271 = (DWORD)gamename;

	// patch build date
	const char* buildDate = "1396/7/5";
	strcpy((char*)0x6F1BB0, buildDate);

	// load ini
	const char* serverName = reader.ReadString("Configuration", "Server", "server.iw4.ir");//"log1.pc.iw4.iwnet.infinityward.com"

	// max_dropped_weapons command
	*(BYTE*)0x5E640F = 0;

	// sv_network_fps max 1000, and uncheat
	*(BYTE*)0x4BBF79 = 0; // ?
	*(DWORD*)0x4BBF7B = 1000;

	// remove fs_game stats
	*(WORD*)0x48D9C4 = 0x9090;

	// make statWriteNeeded ignored
	//*(BYTE*)0x424BD0 = 0xEB;

	// remove 'impure stats' notification
	*(BYTE*)0x501A30 = 0x33;
	*(BYTE*)0x501A31 = 0xC0;
	*(DWORD*)0x501A32 = 0xC3909090;

	// patch web1 server
	const char* webName = reader.ReadString("Configuration", "WebHost", "http://web1.pc.iw4.iwnet.infinityward.com/pc/");

	if (!strcmp(webName, "auto")) {
		sprintf(webServerName, "http://%s:%d/pc/", serverName, 13000);
	} else {
		// we don't want a buffer overrun, do we?
		if (strlen(webName) < 1024) {
			memcpy(webServerName, webName, 1024);
		}
	}

	*(DWORD*)0x4CD220 = (DWORD)&webServerName;
	*(DWORD*)0x4CD23F = (DWORD)&webServerName;

	// exec unlock
	//*(BYTE*)0x60C017 = 0xEB;

	// de-cheat cg_fov
	*(BYTE*)0x4E1115 = 1;//40;

	// set cg_fov max to 120.0
	//float cgFov90 = 120.0f;
	//*(DWORD*)0x4E1108 = (DWORD)&cgFov90;

	// fs_game crash fix: disable some useless stuff
	*(BYTE*)0x481F1D = 0xEB;
	//*(WORD*)0x481F1D = 0x9090;

	// DLC1 test
	//const char* playlists_dlc1_ff = "playlists_allmaps.info";
	//*(DWORD*)0x60D73B = (DWORD)playlists_dlc1_ff;

	//strcpy((char*)0x709D8C, "mp_playlists_dlc1");

	// patch .ff protection
	/**(BYTE*)0x414AB9 = 0xEB;
	*(BYTE*)0x4975B9 = 0xEB;
	*(BYTE*)0x5BD747 = 0xEB;

	*(BYTE*)0x50B3E7 = 0xEB;
	*(WORD*)0x50B3C7 = 0x9090;

	*(BYTE*)0x4975C7 = 0xEB;*/

	//*(BYTE*)0x5BD14D = 0xEB;
	//*(WORD*)0x5BD14D = 0x9090;
	//*(WORD*)0x5BD164 = 0x9090;
	//*(WORD*)0x5BD17B = 0x9090;
	//*(WORD*)0x5BD185 = 0x9090;

	//*(BYTE*)0x5BD38F = 0xEB;
	//*(WORD*)0x5BD38F = 0x9090;
	//*(WORD*)0x5BD3A6 = 0x9090;
	//*(WORD*)0x5BD3BD = 0x9090;
	//*(WORD*)0x5BD3C7 = 0x9090;

	//*(WORD*)0x5BDD02 = 0x9090;
	//*(WORD*)0x5BDCBC = 0x9090;

	// for playlists
	//*(BYTE*)0x5BDD02 = 0xEB;
	//*(BYTE*)0x5BDCBC = 0xEB;

	// for others
	//*(BYTE*)0x5BD502 = 0xEB;
	//*(BYTE*)0x5BD4BC = 0xEB;

	*(WORD*)0x5BD502 = 0x9090;
	*(WORD*)0x5BD4BC = 0x9090;

	*(WORD*)0x5BDD02 = 0x9090;
	*(WORD*)0x5BDCBC = 0x9090;

	*(WORD*)0x5BD343 = 0x9090;
	*(WORD*)0x5BD354 = 0x9090;

	*(WORD*)0x5BDB43 = 0x9090;
	*(WORD*)0x5BDB54 = 0x9090;
	//*(BYTE*)0x5BD343 = 0xEB;
	//*(BYTE*)0x5BD354 = 0xEB;

	//*(BYTE*)0x5BDB43 = 0xEB;
	//*(BYTE*)0x5BDB54 = 0xEB;

	// .ff filename hash check (?)
	*(BYTE*)0x5BD4B2 = 0xB8;
	*(DWORD*)0x5BD4B3 = 1;

	*(BYTE*)0x5BD4F8 = 0xB8;
	*(DWORD*)0x5BD4F9 = 1;

	// remove limit on IWD file loading
	memset((void*)0x643B94, 0x90, 6);

	// remove convar write protection
	*(BYTE*)0x647DD4 = 0xEB;

	// remove fs_game check for moddable rawfiles
	*(WORD*)0x61C6D6 = 0x9090;

	// kill filesystem init default_mp.cfg check -- IW made it useless while moving .cfg files to fastfiles
	// and it makes fs_game crash

	// not nopping everything at once, there's cdecl stack cleanup in there
	memset((void*)0x4CDDFE, 0x90, 5);
	memset((void*)0x4CDE0A, 0x90, 5);
	memset((void*)0x4CDE12, 0x90, 0xB1);

	// for some reason fs_game != '' makes the game load mp_defaultweapon, which does not exist in MW2 anymore as a real asset
	// kill the call and make it act like fs_game == ''
	// UPDATE 2010-09-12: this is why CoD4 had text weapon files, those are used with fs_game.
	*(BYTE*)0x43A3ED = 0xEB;

	//*(DWORD*)0x44BE97 = (DWORD)leanLeftPTest;
	//*(DWORD*)0x44BEB0 = (DWORD)leanLeftMTest;

	// HOTFIX: pingPlayer on dedicated servers
	*(BYTE*)0x48B4DA = 0xEB;

	// maxplayers
	/**(BYTE*)0x5E62DC = 32;

	*(BYTE*)0x5E62E0 = 32;
	*(BYTE*)0x5E6301 = 32;
	//
	*(BYTE*)0x42A869 = 32;

	*(BYTE*)0x42A86D = 32;
	*(BYTE*)0x42A889 = 32;
	//
	*(BYTE*)0x4BBA80 = 32;

	*(BYTE*)0x4BBA84 = 32;
	*(BYTE*)0x4BBAA5 = 32;
	//
	*(BYTE*)0x44618E = 32;
	*(BYTE*)0x446192 = 32;
	//
	memset((void*)0x41AED1, 0x90, 5);
	memset((void*)0x41AEDE, 0x90, 5);

	// party consistency in SV_DirectConnect
	*(BYTE*)0x49FB36 = 0xEB;*/

	// DEDICATED SERVER TEST
#if DEDICATED
	*(BYTE*)0x5B8A90 = 0xC3; // self-registration on party
	*(BYTE*)0x467560 = 0xC3; // other party stuff?

	//*(BYTE*)0x412300 = 0xC3; // upnp devices

	*(BYTE*)0x4C6803 = 0x04; // make CL_Frame do client packets, even for game state 9
	*(BYTE*)0x4F6780 = 0xC3; // init sound system (1)
	*(BYTE*)0x50E3E0 = 0xC3; // start render thread
	*(BYTE*)0x4AA170 = 0xC3; // R_Init caller
	*(BYTE*)0x439930 = 0xC3; // init sound system (2)
	*(BYTE*)0x49C390 = 0xC3; // Com_Frame audio processor?
	*(BYTE*)0x4CB480 = 0xC3; // called from Com_Frame, seems to do renderer stuff

	*(WORD*)0x423299 = 0x9090; // some check preventing proper game functioning

	memset((void*)0x50E4D9, 0x90, 6); // another similar bsp check

	// steam stuff
	// AnonymousAddRequest
	*(WORD*)0x5B98FC = 0x9090;
	*(BYTE*)0x5B9904 = 0xEB;

	// HandleClientHandshake
	*(WORD*)0x5BA9CB = 0x9090;
	*(BYTE*)0x5BA9D3 = 0xEB;

	// R_Init caller + more
	//memset((void*)0x4AA170, 0x90, 5);
	//*(BYTE*)0x4AA17F = 0xC3;

	*(BYTE*)0x520547 = 0; // r_loadForRenderer default to 0

	// disable cheat protection on onlinegame
	*(BYTE*)0x4926A7 = 0x80; 


	//*(BYTE*)0x4DF670 = 0xC3; // mp_playlists loader
	//*(BYTE*)0x4C5AD0 = 0xC3; // CL_Init
	//*(BYTE*)0x492650 = 0xC3; // CL commands

	*(BYTE*)0x50EDE0 = 0xC3; // some d3d9 call on error

	// load fastfiles independtly from renderer
	dedicatedInitHook.initialize("aaaaa", (PBYTE)dedicatedInitHookLoc);
	dedicatedInitHook.installHook(DedicatedInitHookStub, false);
#endif

	// possible fix for allowing alt-tab (r_autopriority kills WM_KILLFOCUS and similar)
	//*(BYTE*)0x42AEEF = 0xEB;
	//*(BYTE*)0x42AF15 = 0xEB;
	/**(BYTE*)0x50FE90 = 0xC3;
	*(BYTE*)0x463F40 = 0xC3;
	*(BYTE*)0x49C390 = 0xC3;
	*(BYTE*)0x4AA170 = 0xC3;
	*(BYTE*)0x439930 = 0xC3;
	*(BYTE*)0x43EEE0 = 0xC3;
	*(BYTE*)0x60E5D0 = 0xC3;
	memset((void*)0x50E6D2, 0x90, 5);
	memset((void*)0x4429F2, 0x90, 5);
	memset((void*)0x60E4F2, 0x90, 5);*/

	// B0 01
	//*(WORD*)0x5BD31C = 0x1B0;

	//memset((void*)0x5BD270, 0x90, 5);
	//memset((void*)0x5BDA81, 0x90, 5);

	/*memset((void*)0x5BD339, 0x90, 5);
	memset((void*)0x5BD4B2, 0x90, 5);
	memset((void*)0x5BD4F8, 0x90, 5);

	memset((void*)0x5BDB39, 0x90, 5);
	memset((void*)0x5BDCB2, 0x90, 5);
	memset((void*)0x5BDCF8, 0x90, 5);*/

	//InitializeValidation();

	// REALLOCATION
	ReallocTitties();  // entities
	//ReallocHardCoil(); // recoil
	//IranRev New Changes Here (UnComment)
}

//void* newEnts;
BYTE newEnts[516 * 2048];
//float* newCoil;
/*
void ReallocHardCoil() {
// allocate new memory
int newsize = 32;
newCoil = (float*)malloc(newsize);

// get (obfuscated) memory locations
// 8686D4?
unsigned int origMin = 0x94B2AA ^ 0x123456; // 8686FC ; formerly F0
unsigned int origMax = 0x9E9E26 ^ 0x181922; // 868704

unsigned int difference = (unsigned int)newCoil - origMin;

// scan the .text memory
char* scanMin = (char*)0x401000;
char* scanMax = (char*)0x6D5B8A;
char* current = scanMin;

for (; current < scanMax; current += 1) {
unsigned int* intCur = (unsigned int*)current;

// if the address points to something within our range of interest
if (*intCur >= origMin && *intCur <= origMax) {
// patch it
*intCur += difference;
}
}	
}*/

void ReallocTitties() {
	// allocate new memory
	int newsize = 516 * 2048;
	//newEnts = malloc(newsize);

	// get (obfuscated) memory locations
	unsigned int origMin = 0x42B7B5 ^ 0xCDCDCD; // 8F7A78
	unsigned int origMax = 0x9D6E6E ^ 0x121212; // 8F7C7C

	unsigned int difference = (unsigned int)newEnts - origMin;

	// scan the .text memory
	char* scanMin = (char*)0x401000;
	char* scanMax = (char*)0x6D5B8A;
	char* current = scanMin;

	for (; current < scanMax; current += 1) {
		unsigned int* intCur = (unsigned int*)current;

		// if the address points to something within our range of interest
		if (*intCur >= origMin && *intCur <= origMax) {
			// patch it
			*intCur += difference;
		}
	}

	// give them a message they'll never forget
	const char* message = "HELLO, DEAR <<CHEAT DEVELOPER NAME HERE>>. THIS MEMORY LOCATION WAS ONCE FILLED WITH 1 MB OF GOODNESS FOR YOU, BUT NOW IT'S EMPTY. WHY, YOU'D SAY? WELL, I'LL GIVE YOU A HINT: IT'S LOCKED AWAY SAFELY UNTIL THE CODE NEEDS THIS MEMORY DATA. WHEN THAT EXECUTES, YOU CAN HOOK IN, AND READ ALL YOU WANT OF THIS LOVELY DATA SECTION. ALSO, I WANT YOU TO POST A PICTURE OF YOUR LITTLE, CHILDISH FACE ONCE YOU NOTICE THIS MESSAGE (MOST OF YOU ARE LI'L KIDS, RIGHT?) TOGETHER WITH THIS MESSAGE YOURSELF ON <<CHEATING SITE NAME HERE>>. DON'T WORRY, I'LL GOOGLE IT. ALSO, AS SOME OF YOU HAVE FAILED TO KEEP THE WHITE FLAG UP FOR TOO LONG, I THINK IT'S SAFE TO SAY THE ARMS RACE CONTINUES. GOOD LUCK HACKING, OR IF YOU'RE SMART, YOU MOVE ON TO ANOTHER GAME. -- NTA ;)";
	strcpy((char*)origMin, message);

	// and that should be it?
}

#pragma managed
extern bool connectedInterface;

void ErrorWithWebLink(String^ error, String^ webLink) {
	MessageBox::Show(error, "Iran Revolution", MessageBoxButtons::OK, MessageBoxIcon::Error);

	Process::Start(webLink);
}

void WarnOut(String^ message) {
	MessageBox::Show(message, "Iran Revolution", MessageBoxButtons::OK, MessageBoxIcon::Warning);
}


/*!
\brief Check if a process is running
\param [in] processName Name of process to check if is running
\returns \c True if the process is running, or \c False if the process is not running
*/
bool IsProcessRunning(const wchar_t *processName)
{
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!wcsicmp(entry.szExeFile, processName))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}

void RSAProc_Initialize(String^ server);

char easytolower(char in){
	if(in<='Z' && in>='A')
		return in-('Z'-'z');
	return in;
} 

bool VerifyMW2() {

	try {
		// check 0: is this launched using the updater?
#if !DEDICATED
		if (!IsDebuggerPresent()) {
			// trust most people don't manually CreateProcess
			HMODULE iw4mp = GetModuleHandleA("iw4mp.exe");//"iw4mp.dat" ,"IranRev.dat"
			HMODULE iw4sp = GetModuleHandleA("iw4sp.exe");

			if (!iw4sp && !iw4mp) {
				return false;
			}
		}

		// check 1: iw_22.iwd
		FileInfo^ iw22 = gcnew FileInfo("main\\iw_22.iwd");

		if (iw22->Length != 149435968 || !connectedInterface) {
			ErrorWithWebLink("The main\\iw_22.iwd file is incorrect. You might have used the 'Update' package while having an old version.\nClick OK to be directed to the alterIWnet download page.", "http://alteriw.net/viewtopic.php?f=3&t=24");
			return false;
		}

		// check 2: non-english game
		bool englishExists = Directory::Exists("zone\\english");
		bool nonEnglishExists = (Directory::Exists("zone\\german") || Directory::Exists("zone\\french") || Directory::Exists("zone\\russian") || Directory::Exists("zone\\japanese") || Directory::Exists("zone\\polish") || Directory::Exists("zone\\italian") || Directory::Exists("zone\\spanish"));

		/*(if (nonEnglishExists) {
		WarnOut("Non-English MW2 language files have been detected. This may cause problems with connecting to servers. Only English game versions are officially supported by IranRev.");
		}*/

		// check 2.5: incorrect iw4mp.dat
		System::Security::Cryptography::SHA1Managed^ sha1 = gcnew System::Security::Cryptography::SHA1Managed();
		FileStream^ iw4 = File::OpenRead("iw4mp.exe");//"iw4mp.dat" ,"IranRev.dat"
		cli::array<Byte>^ sha1hash = sha1->ComputeHash(iw4);
		iw4->Close();

		cli::array<Byte>^ comparison = gcnew cli::array<Byte>(20) { 0x3a, 0x3e, 0x6b, 0xed, 0x46, 0x72, 0x6c, 0x91, 0x86, 0xf5, 0x15, 0x65, 0xac, 0x1d, 0xe8, 0x01, 0x60, 0x0f, 0xc5, 0x94 };

		bool isEqual = true;

		for (int i = 0; i < comparison->Length; i++) {
			if (sha1hash[i] != (comparison[i] ^ 0x5D)) {
				isEqual = false;
				break;
			}
		}

		/*if (!isEqual) {
		WarnOut("Non-Australian MW2 language files have been detected. This may cause problems with connecting to servers. Only English game versions are officially supported by alterIWnet.");
		//return false;
		}*/

		// check 3: patch_mp.ff
		if (englishExists) {
			FileInfo^ patchMP = gcnew FileInfo("zone\\english\\patch_mp.ff");

			if (patchMP->Length != 213013) {
				//ErrorWithWebLink("The zone\\english\\patch_mp.ff file is incorrect. You might have incorrectly overwritten these files.\nClick OK to be directed to the alterIWnet download page.", "http://alteriw.net/viewtopic.php?f=3&t=24");
				//return false;
			}
		}

		// check 6: server available?
		CIniReader reader(".\\IranRev.ini");
		const char* serverName = reader.ReadString("Configuration", "Server", "server.iw4.ir"/*"log1.pc.iw4.iwnet.infinityward.com"*/);

		String^ hostName = gcnew String(serverName);
		Sockets::Socket^ client = gcnew Sockets::Socket(Sockets::AddressFamily::InterNetwork, Sockets::SocketType::Stream, Sockets::ProtocolType::Tcp);
		client->Blocking = false;

		bool hostUp = true;

		try {
			client->Connect(hostName, 13000);
		} catch (Sockets::SocketException^ ex) {
			if (ex->NativeErrorCode != 10035) {
				hostUp = false;
			} else {
				if (!client->Poll(3000 * 1000, Sockets::SelectMode::SelectWrite)) {
					hostUp = false;
				}
			}
		} finally {
			client->Close();
		}

		if (!hostUp || (hostName->Contains("7") && hostName->EndsWith("t"))) {
			WarnOut("Could not connect to the configured server (" + hostName + "). Possibly you've misconfigured IranRev.ini, or this server is actually down.");
			//return false;
		}

		bool isWarningShown = reader.ReadBoolean("Dynamic", "ServerWarningShown", false);

		//if (/*!hostName->Contains("alteriw.net")*/!hostName->Contains("ircod4.ir")||!hostName->Contains("iw4play.ir")||!hostName->Contains("iw4.ir")||!hostName->Contains("server.iw4.ir")||!hostName->Contains("server.iw4play.ir")) {
		if (!hostName->Contains("iw4.ir")) {
			WarnOut("The alterIWnet server you are connecting to (" + hostName + ") is not officially supported by the Iran Revolution project. For any help with server-related issues, use the support of whoever provided this server.");

			reader.WriteBoolean("Dynamic", "ServerWarningShown", true);
		}

		bool isHackWarningShown = reader.ReadBoolean("Dynamic", "HackWarningShown", false);

		if (!isHackWarningShown) {
			Application::Run(gcnew steam_api::HackDisclaimer());

			if (!steam_api::HackDisclaimer::accepted) {
				return false;
			}

			reader.WriteBoolean("Dynamic", "HackWarningShown", true);
		}


		// check 7: player has name?

		const char* nickname = reader.ReadString("Configuration", "Nickname", "");

		//easytolower(const_cast<char *>(nickname));

		if (strlen(nickname)>80 || strlen(nickname) == 0 || nickname == ""|| nickname == "Unknown Player"|| nickname == "Name Here"|| nickname == "Your Name Here"|| nickname == "NameHere"|| nickname == "Change Your Name") {
			Application::Run(gcnew steam_api::NameChanger());

			if (!steam_api::NameChanger::accepted) 
				reader.WriteString("Configuration", "Nickname", "Unknown Player");
			else
				reader.WriteString("Configuration", "Nickname", steam_api::NameChanger::NickName);
		}
		// Show Splash Logo
		//ShowLogo();

		RSAProc_Initialize(hostName);
#endif

		return true;
	} catch (Exception^ e) {
		WarnOut(e->ToString());
		return false;
	}
}

public ref class HiddenWTF {
public:
	static void SayToAll(Drawing::Color color, String^ name, String^ text) {
		IntPtr n = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(name);
		IntPtr t = Runtime::InteropServices::Marshal::StringToHGlobalAnsi(text);

		//G_SayToAll(color.ToArgb(), (char*)n.ToPointer(), (char*)t.ToPointer());
		CG_PrintChat((char*)t.ToPointer());

		Runtime::InteropServices::Marshal::FreeHGlobal(n);
		Runtime::InteropServices::Marshal::FreeHGlobal(t);
	}

	static property String^ NickName {
		String^ get() {
			CIniReader reader(".\\IranRev.ini");
			const char* name = reader.ReadString("Configuration", "Nickname", "[IranRev] Unnamed");

			return gcnew String(name);
		}
	}
};