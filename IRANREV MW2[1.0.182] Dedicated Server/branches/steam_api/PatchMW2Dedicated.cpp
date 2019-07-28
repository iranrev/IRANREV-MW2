// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: dedicated servers
//
// Initial author: NTAuthority
// Started: 2010-09-27
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"
#include "hwbrk.h"
#include "AdminPlugin.h"

#include <winsock2.h>
#include <CrashRpt.h>

#pragma unmanaged

typedef void (__cdecl * CommandCB_t)(void);

typedef void (*LoadInitialFF_t)(void);
LoadInitialFF_t LoadInitialFF = (LoadInitialFF_t)0x50D280;

typedef int (__cdecl * RegStringDvar_t)(const char*, const char*, int, const char*);
RegStringDvar_t RegStringDvar = (RegStringDvar_t)0x4A0C70;

typedef void (__cdecl * RegisterCommand_t)(const char* name, CommandCB_t callback, cmd_t* data, char);
RegisterCommand_t RegisterCommand = (RegisterCommand_t)0x50ACE0;

typedef void (__cdecl * RegisterServerCommand_t)(const char* name, CommandCB_t callback, cmd_t* data);
RegisterServerCommand_t RegisterServerCommand = (RegisterServerCommand_t)0x449FA0;

typedef cvar_t* (__cdecl * RegIntDvar_t)(const char* name, int default, int min, int max, int flags, const char* description);
RegIntDvar_t RegIntDvar = (RegIntDvar_t)0x423BC0;

CommandCB_t __serverCommand = (CommandCB_t)0x5BE280;

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

cmd_t sv_tell;
cmd_t sv_tell2;

cmd_t sv_say;
cmd_t sv_say2;

cmd_t sv_maprotate;
cmd_t sv_maprotate2;

cmd_t cv_sets;

void SV_Tell_f();
void SV_Say_f();
void SV_MapRotate_f();
void Cvar_SetS_f();

cvar_t* aiw_sayName;
cvar_t* aiw_username;
cvar_t* aiw_password;

cvar_t* sv_mapRotation;
cvar_t* sv_mapRotationCurrent;
cvar_t* aiw_version;

cvar_t* aiw_remoteKick;
cvar_t* aiw_secure;

void InitDedicatedVars() {
	*(DWORD*)0x633A328 = RegStringDvar("ui_gametype", "", 0, "Current game type");
	*(DWORD*)0x633A29C = RegStringDvar("ui_mapname", "", 0, "Current map name");
	sv_rconPassword = (cvar_t*)RegStringDvar("rcon_password", "", 0, "[aIW] the password for rcon");
	aiw_sayName = (cvar_t*)RegStringDvar("aiw_sayName", "^7Console", 0, "[aIW] the name to pose as for 'say' commands");
	aiw_username = (cvar_t*)RegStringDvar("aiw_username", "", 0, "[aIW] username for +features");
	aiw_password = (cvar_t*)RegStringDvar("aiw_password", "", 0, "[aIW] password for +features");
	sv_mapRotation = (cvar_t*)RegStringDvar("sv_mapRotation", "map_restart", 0, "List of maps for the server to play");
	sv_mapRotationCurrent = (cvar_t*)RegStringDvar("sv_mapRotationCurrent", "", 0, "Current map in the map rotation");
	aiw_version = (cvar_t*)RegStringDvar("aiw_version", VERSION, 0x2000, "");

	aiw_secure = (cvar_t*)RegIntDvar("aiw_secure", 1, 0, 1, 1024, "Enable checking of 'clean' client status");
	aiw_remoteKick = (cvar_t*)RegIntDvar("aiw_remoteKick", 1, 0, 1, 1024, "Allow the master server to kick unclean clients automatically");

	RegisterCommand("say", __serverCommand, &sv_say, 0);
	RegisterServerCommand("say", SV_Say_f, &sv_say2);

	RegisterCommand("tell", __serverCommand, &sv_tell, 0);
	RegisterServerCommand("tell", SV_Tell_f, &sv_tell2);

	RegisterCommand("sets", Cvar_SetS_f, &cv_sets, 0);

	//RegisterCommand("map_rotate", __serverCommand, &sv_maprotate, 0);
	//RegisterServerCommand("map_rotate", SV_MapRotate_f, &sv_maprotate2);
}

void __declspec(naked) DedicatedInitHookStub() {
	InitDedicatedFastFiles();
	InitDedicatedVars();

	__asm {
		jmp dedicatedInitHook.pOriginal
	}
}

DWORD gSayPreHookLoc = 0x4150EB;
CallHook gSayPreHook;

DWORD gSayPostHook1Loc = 0x4151B4;
CallHook gSayPostHook1;

DWORD gSayPostHook2Loc = 0x4151F0;
CallHook gSayPostHook2;

bool gsShouldSend = true;
char* gspText;
char* gspName;
void* gspEnt;

void __declspec(naked) GSayPreHookFunc()
{
	__asm mov eax, [esp + 100h + 10h]
	__asm mov gspText, eax

	__asm mov eax, [esp + 4h] // as name is arg to this function, that should work too
	__asm mov gspName, eax

	__asm mov eax, [esp + 100h + 4h]
	__asm mov gspEnt, eax

	gsShouldSend = true;

	if (APC_TriggerSay(gspEnt, gspName, &gspText[1]))
	{
		gsShouldSend = false;
	}
	else if (gspText[1] == '/')
	{
		gsShouldSend = false;

		gspText[1] = gspText[0];
		gspText += 1;
		__asm mov eax, gspText
		__asm mov [esp + 100h + 10h], eax
	}

	__asm jmp gSayPreHook.pOriginal
}

// these two need to pushad/popad as otherwise some registers the function uses as param are screwed up
void __declspec(naked) GSayPostHook1Func()
{
	__asm pushad

	if (!gsShouldSend)
	{
		__asm popad
		__asm retn
	}

	__asm popad

	__asm jmp gSayPostHook1.pOriginal
}

void __declspec(naked) GSayPostHook2Func()
{
	__asm pushad

	if (!gsShouldSend)
	{
		__asm popad
		__asm retn
	}

	__asm popad

	__asm jmp gSayPostHook2.pOriginal
}

CallHook gRunFrameHook;
DWORD gRunFrameHookLoc = 0x62858D;

void G_RunFrameHookFunc()
{
	APC_TriggerFrame();
}

void __declspec(naked) G_RunFrameHookStub()
{
	G_RunFrameHookFunc();

	__asm
	{
		jmp gRunFrameHook.pOriginal
	}
}

StompHook doPostInitHook1;
CallHook steamCheckHook;
DWORD doPostInitHook1Loc = 0x60E64F;
DWORD cliCommandHook1Loc = 0x4777B1;
DWORD cliCommandHook2Loc = 0x4777D4;
DWORD steamCheckHookLoc = 0x40F7CA;

typedef void (__cdecl * Cmd_ExecuteString_t)(int a1, int a2, char* cmd);
extern Cmd_ExecuteString_t Cmd_ExecuteString;

void PushAutoCommands()
{
	Cmd_ExecuteString(0, 0, "exec autoexec.cfg");
	Cmd_ExecuteString(0, 0, "onlinegame 1");
	Cmd_ExecuteString(0, 0, "exec default_xboxlive.cfg");
	Cmd_ExecuteString(0, 0, "xblive_rankedmatch 0");
	Cmd_ExecuteString(0, 0, "xblive_privatematch 1");
	Cmd_ExecuteString(0, 0, "xstartprivatematch");
	Cmd_ExecuteString(0, 0, "sv_network_fps 1000");
	Cmd_ExecuteString(0, 0, "com_maxfps 0");
}

DWORD processCLI = 0x60EA60;
DWORD doPostInit = 0x40A8B0;

void __declspec(naked) DoPostInitStub()
{
	__asm
	{
		call PushAutoCommands
		call processCLI
		jmp doPostInit
	}
}

void __declspec(naked) CLICommandHook2Stub()
{
	__asm
	{
		call PushAutoCommands
		jmp processCLI
	}
}

static bool validated = false;
static bool isValid = false;

typedef void (__cdecl * Cbuf_AddText_t)(int, char*);
extern Cbuf_AddText_t Cbuf_AddText;

typedef void (__cdecl * Com_Printf_t)(int, const char*, ...);
extern Com_Printf_t Com_Printf;

#pragma managed
using namespace System;
using namespace System::Collections::Specialized;
using namespace System::Net;

void DoSteamCheck()
{
	// check my keys, you'll see that I've not been pirating this private version of aIW!
	if (!validated)
	{
		bool valid = false;

		/*if (aiw_username->value.string && strlen(aiw_username->value.string))
		{
			if (aiw_password->value.string && strlen(aiw_password->value.string))
			{
				String^ username = gcnew String(aiw_username->value.string);
				String^ password = gcnew String(aiw_password->value.string);

				ServicePointManager::Expect100Continue = false;

				NameValueCollection^ data = gcnew NameValueCollection();
				data->Add("username", username);
				data->Add("password", password);

				WebClient^ client = gcnew WebClient();
				String^ txt = Text::Encoding::UTF8->GetString(client->UploadValues("http://alteriw.net/verify.php", data));

				IntPtr ptr = Runtime::InteropServices::Marshal::StringToHGlobalAnsi("aIW auth returned " + txt + "\n");
				Com_Printf(0, (char*)ptr.ToPointer());
				Runtime::InteropServices::Marshal::FreeHGlobal(ptr);

				/*float value = 3.14f;
				bool is = float::TryParse(txt, value);

				if (is)
				{
					if ((value + 1.0f) == 14.37f)
					{
						valid = true;
					}
				}* /

				if (txt->Trim() == "13.37")
				{
					valid = true;
				}
			}
		}*/

		ServicePointManager::Expect100Continue = false;

		NameValueCollection^ data = gcnew NameValueCollection();
		data->Add("version", VERSION);

		try
		{
			WebClient^ client = gcnew WebClient();
			//String^ txt = Text::Encoding::UTF8->GetString(client->UploadValues("http://alteriw.net/verifyDedi.php", data));
			String^ txt = Text::Encoding::UTF8->GetString(client->UploadValues("http://iranrev.iw4play.ir/verifyDedi.php", data));

			IntPtr ptr = Runtime::InteropServices::Marshal::StringToHGlobalAnsi("aIW auth returned " + txt + "\n");
			Com_Printf(0, (char*)ptr.ToPointer());
			Runtime::InteropServices::Marshal::FreeHGlobal(ptr);

			//if (txt->Trim() == "13.37")
			if (txt->Trim() == "13.33")
			{
				valid = true;
			}
		}
		catch (Exception^)
		{
			valid = true;
		}

		validated = true;
		isValid = valid;
	}

	if (!isValid)
	{
		BYTE* clientAddress = (BYTE*)0x3230E90;

		//strncpy(infostring, ConsoleToInfo(1028), 8192);

		for (int i = 0; i < *(int*)0x3230E8C; i++)
		{
			if (*clientAddress >= 5)
			{ // connected
				Com_Printf(0, "A client was kicked due to aIW validation failing. This might mean you need to update your dedicated server installation. Please see http://alteriw.net/dedicate for details.\n");

				char kickme[256];
				sprintf(kickme, "clientkick %d PLATFORM_STEAM_CONNECT_FAIL\n", i);

				Cbuf_AddText(0, kickme);
			}

			clientAddress += 681872;
		}

	}
}
#pragma unmanaged

void DoSteamCheckWrap()
{
	DoSteamCheck();
}

void __declspec(naked) DoSteamCheckStub()
{
	__asm push ecx
	DoSteamCheckWrap();
	__asm pop ecx
	__asm jmp steamCheckHook.pOriginal
	//__asm retn
}

CallHook heartbeatHookFrame;
DWORD heartbeatHookFrameLoc = 0x6289B1;

StompHook svheartbeatHook;
DWORD svheartbeatHookLoc = 0x417D80;

CallHook killServerHook;
DWORD killServerHookLoc = 0x43EF28;

CallHook stopServerHook;
DWORD stopServerHookLoc = 0x4B6145;

void SV_Heartbeat_f();

void __declspec(naked) Hook_SV_Heartbeat_f()
{
	__asm jmp SV_Heartbeat_f
}

void SV_MasterHeartbeat();

void __declspec(naked) HeartbeatHookFrameStub()
{
	SV_MasterHeartbeat();
	__asm jmp heartbeatHookFrame.pOriginal
}

void SV_MasterHeartbeat_OnKill();

void __declspec(naked) KillServerHookStub()
{
	SV_MasterHeartbeat_OnKill();
	__asm jmp killServerHook.pOriginal
}

void __declspec(naked) StopServerHookStub()
{
	SV_MasterHeartbeat_OnKill();
	__asm jmp stopServerHook.pOriginal
}

CallHook checkPrivateSlotHook;
DWORD checkPrivateSlotHookLoc = 0x5BAC5B;

int CheckPrivateSlotHookFunc(void* unknown, __int64 guid)
{
	int retval = -1;
	FILE* slots = fopen("reservedslots.txt", "r");

	if (!slots)
	{
		return retval;
	}

	while (!feof(slots))
	{
		int testGuid;
		if (fscanf(slots, "%lx\n", &testGuid) > 0)
		{
			if (testGuid == (guid & 0xFFFFFFFF))
			{
				retval = 1;
				break;
			}
		}
		else
		{
			break;
		}
	}

	fclose(slots);
	return retval;
}

void __declspec(naked) CheckPrivateSlotHookStub()
{
	__asm jmp CheckPrivateSlotHookFunc
}

#define NEW_MAXCLIENTS 24

void ReallocStuff();

void PatchMW2_Dedicated()
{
	*(BYTE*)0x5B8A90 = 0xC3; // self-registration on party
	*(BYTE*)0x467560 = 0xC3; // other party stuff?

	//*(BYTE*)0x412300 = 0xC3; // upnp devices
	*(BYTE*)0x4BA2B0 = 0xC3; // upnp stuff
	
	*(BYTE*)0x4C6803 = 0x04; // make CL_Frame do client packets, even for game state 9
	*(BYTE*)0x4F6780 = 0xC3; // init sound system (1)
	*(BYTE*)0x50E3E0 = 0xC3; // start render thread
	*(BYTE*)0x4AA170 = 0xC3; // R_Init caller
	*(BYTE*)0x439930 = 0xC3; // init sound system (2)
	*(BYTE*)0x49C390 = 0xC3; // Com_Frame audio processor?
	*(BYTE*)0x4CB480 = 0xC3; // called from Com_Frame, seems to do renderer stuff

	*(BYTE*)0x4982F0 = 0xC3; // some mixer-related function called on shutdown

	*(BYTE*)0x60D45D = 0;    // masterServerName flags

	*(WORD*)0x423299 = 0x9090; // some check preventing proper game functioning

	memset((void*)0x50E4D9, 0x90, 6); // another similar bsp check

	memset((void*)0x4A417D, 0x90, 6); // unknown check in SV_ExecuteClientMessage (0x20F0890 == 0, related to client->f_40)

	memset((void*)0x4CAA5D, 0x90, 5); // function checking party heartbeat timeouts, causes random issues

	memset((void*)0x4232B9, 0x90, 5); // some deinit renderer function

	memset((void*)0x5A0896, 0x90, 5); // warning message on a removed subsystem
	memset((void*)0x45DE3F, 0x90, 5); // same as above

	// map_rotate func
	*(DWORD*)0x430708 = (DWORD)SV_MapRotate_f;

	// sv_network_fps max 1000, and uncheat
	*(BYTE*)0x4BBF79 = 0; // ?
	*(DWORD*)0x4BBF7B = 1000;

	// steam stuff
	*(DWORD*)0x46FB8C = 0x90909090;
	*(WORD*)0x46FB90 = 0x9090;

	// AnonymousAddRequest
	*(BYTE*)0x5B98B8 = 0xEB;
	*(WORD*)0x5B98FC = 0x9090;
	*(BYTE*)0x5B9904 = 0xEB;

	// HandleClientHandshake
	*(BYTE*)0x5BA985 = 0xEB;
	*(WORD*)0x5BA9CB = 0x9090;
	*(BYTE*)0x5BA9D3 = 0xEB;

	*(BYTE*)0x520547 = 0; // r_loadForRenderer default to 0
	
	// disable cheat protection on onlinegame
	*(BYTE*)0x4926A7 = 0x80; 

	*(BYTE*)0x50EDE0 = 0xC3; // some d3d9 call on error

	// load fastfiles independtly from renderer
	dedicatedInitHook.initialize("dedia", (PBYTE)dedicatedInitHookLoc);
	dedicatedInitHook.installHook(DedicatedInitHookStub, false);

	// slash command stuff
	gSayPreHook.initialize("dedia", (PBYTE)gSayPreHookLoc);
	gSayPreHook.installHook(GSayPreHookFunc, false);

	gSayPostHook1.initialize("dedia", (PBYTE)gSayPostHook1Loc);
	gSayPostHook1.installHook(GSayPostHook1Func, false);

	gSayPostHook2.initialize("dedia", (PBYTE)gSayPostHook2Loc);
	gSayPostHook2.installHook(GSayPostHook2Func, false);

	gRunFrameHook.initialize("aaaaa", (PBYTE)gRunFrameHookLoc);
	gRunFrameHook.installHook(G_RunFrameHookStub, false);

	doPostInitHook1.initialize("dedia", 5, (PBYTE)doPostInitHook1Loc);
	doPostInitHook1.installHook(DoPostInitStub, true, false);

	steamCheckHook.initialize("dedia", (PBYTE)steamCheckHookLoc);
	steamCheckHook.installHook(DoSteamCheckStub, false);

	heartbeatHookFrame.initialize("dedia", (PBYTE)heartbeatHookFrameLoc);
	heartbeatHookFrame.installHook(HeartbeatHookFrameStub, false);

	svheartbeatHook.initialize("dedia", 5, (PBYTE)svheartbeatHookLoc);
	svheartbeatHook.installHook(Hook_SV_Heartbeat_f, true, false);

	killServerHook.initialize("dedia", (PBYTE)killServerHookLoc);
	killServerHook.installHook(KillServerHookStub, false);

	stopServerHook.initialize("dedia", (PBYTE)stopServerHookLoc);
	stopServerHook.installHook(StopServerHookStub, false);

	checkPrivateSlotHook.initialize("jerbob is an egghead", (PBYTE)checkPrivateSlotHookLoc);
	checkPrivateSlotHook.installHook(CheckPrivateSlotHookStub, false);

	//cliCommandHook2.initialize("dedia", (PBYTE)cliCommandHook2Loc);
	//cliCommandHook2.installHook(CLICommandHook2Stub, false);
	memset((void*)cliCommandHook1Loc, 0x90, 5);
	memset((void*)cliCommandHook2Loc, 0x90, 5);

	// cause Steam auth to always be requested
	memset((void*)0x46FB8C, 0x90, 6);

	// do not reset sv_privateClients on starting a private match
	memset((void*)0x48245F, 0x90, 5);

	// maxplayers attempt
	/**(BYTE*)0x44618E = NEW_MAXCLIENTS;
	*(BYTE*)0x446192 = NEW_MAXCLIENTS;

	*(BYTE*)0x4BBA80 = NEW_MAXCLIENTS;
	*(BYTE*)0x4BBA84 = NEW_MAXCLIENTS;
	*(BYTE*)0x4BBAA5 = NEW_MAXCLIENTS;

	*(BYTE*)0x42A869 = NEW_MAXCLIENTS;
	*(BYTE*)0x42A86D = NEW_MAXCLIENTS;
	*(BYTE*)0x42A889 = NEW_MAXCLIENTS;

	*(BYTE*)0x5E62DC = NEW_MAXCLIENTS;
	*(BYTE*)0x5E62E0 = NEW_MAXCLIENTS;
	*(BYTE*)0x5E6301 = NEW_MAXCLIENTS;

	// copy to client branch as well
	*(BYTE*)0x5AFFC0 = NEW_MAXCLIENTS;*/

	ReallocStuff();
}

void ReallocPlayerArray1() {
	// allocate new memory
	int newsize = 1324 * NEW_MAXCLIENTS;//8192;//516 * 2048;
	void* newSV = malloc(newsize);

	memset(newSV, 0, newsize);

	// get (obfuscated) memory locations
	unsigned int origMin = 0x1A97988; // 8F7A78
	unsigned int origMax = 0x1A97EB4;//A3;

	unsigned int difference = (unsigned int)newSV - origMin;

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

		if (*intCur == 0x1A9DB9C)
		{
			*intCur += difference;
		}
	}

	*(DWORD*)0x5E7C38 = ((DWORD)newSV) + newsize;
}

// 0x1ADAB28, 13932

void ReallocPlayerArray2() {
	// allocate new memory
	int newsize = 13932 * NEW_MAXCLIENTS;//8192;//516 * 2048;
	void* newSV = malloc(newsize);

	memset(newSV, 0, newsize);

	// get (obfuscated) memory locations
	unsigned int origMin = 0x1ADAB28;
	unsigned int origMax = 0x1ADE194;

	unsigned int difference = (unsigned int)newSV - origMin;

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
}

// 0x3230E90, 681872 (yes, that's 681 kB. IW, what were you smoking while coding this?)

void ReallocPlayerArray3() {
	// allocate new memory
	int newsize = 681872 * NEW_MAXCLIENTS;	// oh, nice, that's 20 MB of allocated memory or so
	void* newSV = malloc(newsize);

	memset(newSV, 0, newsize);				// make that *committed* memory

	// get memory locations
	unsigned int origMin = 0x3230E90;
	unsigned int origMax = 0x32D7620;

	unsigned int difference = (unsigned int)newSV - origMin;

	// scan the .text memory
	char* scanMin = (char*)0x401000;
	char* scanMax = (char*)0x6D5B8A;
	char* current = scanMin;

	for (; current < scanMax; current += 1) {
		unsigned int* intCur = (unsigned int*)current;

		/*unsigned char opcode = *(current - 1);
		unsigned char oldOpcode = *(current - 2);

		if (oldOpcode != 0x81 && oldOpcode != 0x83 && oldOpcode != 0x8B && oldOpcode != 0x8D && opcode != 0x2D && opcode != 0xB8 && opcode != 0xB9 && opcode != 0xBA && opcode != 0xBB && opcode != 0xBC && opcode != 0xBD && opcode != 0xBE && opcode != 0xBF)
		{
			continue;
		}*/

		// some exceptions
		DWORD address = (DWORD)current;
		if (address == 0x4E15A7)
		{
			continue;
		}

		if (address == 0x44D451)
		{
			continue;
		}

		bool patch = false;

		// below list generated by an IDA script
		if (*intCur == 0x03230E90) patch = true;
		if (*intCur == 0x03230EA0) patch = true;
		if (*intCur == 0x03230EB8) patch = true;
		if (*intCur == 0x032314EC) patch = true;
		if (*intCur == 0x03251D04) patch = true;
		if (*intCur == 0x03252130) patch = true;
		if (*intCur == 0x03252134) patch = true;
		if (*intCur == 0x03252148) patch = true;
		if (*intCur == 0x03252158) patch = true;
		if (*intCur == 0x0327296C) patch = true;
		if (*intCur == 0x0327297E) patch = true;
		if (*intCur == 0x03272980) patch = true;
		if (*intCur == 0x03272988) patch = true;
		if (*intCur == 0x0327298D) patch = true;
		if (*intCur == 0x03274989) patch = true;
		if (*intCur == 0x03274D89) patch = true;
		if (*intCur == 0x03274D8B) patch = true;
		if (*intCur == 0x03274D8C) patch = true;

		// if the address points to something within our range of interest
		//if (*intCur >= origMin && *intCur <= origMax)
		if (patch)
		{
			// patch it
			//if ((*intCur) % 4 == 0)
			//{
				*intCur += difference;
			//}
		}
	}
}

// 0x631FD28, 68

void ReallocPlayerArray4() {
	// allocate new memory
	int newsize = 68 * NEW_MAXCLIENTS;//8192;//516 * 2048;
	void* newSV = malloc(newsize);

	memset(newSV, 0, newsize);

	// get (obfuscated) memory locations
	unsigned int origMin = 0x631FD28;
	unsigned int origMax = 0x631FD68;

	unsigned int difference = (unsigned int)newSV - origMin;

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
}

void ReallocStuff()
{
	//ReallocPlayerArray1();
	//ReallocPlayerArray2();
	//ReallocPlayerArray3();
	//ReallocPlayerArray4();
}

// DEDICATED SAY CODE
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

void G_SayToClient(int client, DWORD color, const char* name, const char* text) {
	gentity_t* sourceEntity = &entities[0];
	int unknown = 55;
	int mode = 0;

	gentity_t* other = &entities[client];

	G_SayTo(mode, other, sourceEntity, unknown, color, name, text);
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

void Cmd_GetStringSV(int start, char* buffer, int length)
{
	*buffer = 0;

	for (int i = start; i < Cmd_ArgcSV(); i++) {
		if (strlen(Cmd_ArgvSV(i)) > 64)
		{
			break;
		}

		if (strlen(buffer) > (length - 64))
		{
			break;
		}

		strncat( buffer, Cmd_ArgvSV(i), length );

		if (i != (Cmd_ArgcSV() - 1))
		{
			strcat(buffer, " ");
		}
	}	
}

void SV_Say_f()
{
	if (Cmd_ArgcSV() < 2)
	{
		return;
	}

	//char* message = Cmd_ArgvSV(1);
	char message[1024];
	Cmd_GetStringSV(1, message, sizeof(message));
	G_SayToAll(0xFFFFFFFF, aiw_sayName->value.string, message);
}

void SV_Tell_f()
{
	if (Cmd_ArgcSV() < 3)
	{
		return;
	}

	int client = atoi(Cmd_ArgvSV(1));
	char message[1024];
	Cmd_GetStringSV(2, message, sizeof(message));
	G_SayToClient(client, 0xFFFFFFFF, aiw_sayName->value.string, message);
}
// END DEDI SAY

// DEDI MAP ROTATION
typedef void (__cdecl * SetConsoleString_t)(cvar_t* cvar, char* value);
SetConsoleString_t SetConsoleString = (SetConsoleString_t)0x47F840;

typedef void (__cdecl * SetConsole_t)(char* cvar, char* value);
SetConsole_t SetConsole = (SetConsole_t)0x412580;

char cbuf[512];

char* GetStringConvar(char* key);

cvar_t** sv_running = (cvar_t**)0x1B2F2B4;

void SV_ExecuteLastMap()
{
	char* mapname = GetStringConvar("mapname");

	if (!strlen(mapname))
	{
		mapname = "mp_afghan";
	}

	Cmd_ExecuteString(0, 0, va("map %s", mapname));
}

void SV_MapRotate_f()
{
	Com_Printf(0, "map_rotate...\n\n");
	Com_Printf(0, "\"sv_mapRotation\" is: \"%s\"\n\n", sv_mapRotation->value.string);
	Com_Printf(0, "\"sv_mapRotationCurrent\" is: \"%s\"\n\n", sv_mapRotationCurrent->value.string);

	// if nothing, just restart
	if (strlen(sv_mapRotation->value.string) == 0)
	{
		//Cmd_ExecuteString(0, 0, "map mp_afghan\n");
		SV_ExecuteLastMap();
		return;
	}

	// first, check if the string contains nothing
	if (!sv_mapRotationCurrent->value.string[0])
	{
		Com_Printf(0, "setting new current rotation...\n");

		SetConsoleString(sv_mapRotationCurrent, sv_mapRotation->value.string);
	}

	// now check the 'current' string for tokens -- ending or 'map' means quits
	char rotation[8192];
	memset(rotation, 0, sizeof(rotation));
	strncpy(rotation, sv_mapRotationCurrent->value.string, sizeof(rotation));

	char* token = strtok(rotation, " ");
	bool isType = true;
	char* type = "";
	char* value = "";
	bool changedMap = false;

	while (token)
	{
		if (isType)
		{
			type = token;
		}
		else
		{
			value = token;

			if (!strcmp(type, "gametype"))
			{
				Com_Printf(0, "new gametype: %s\n", value);
				SetConsole("g_gametype", value);
			}
			else if (!strcmp(type, "fs_game"))
			{
				Com_Printf(0, "new fs_game: %s\n", value);

				if (!strcmp(value, "\"\"") || !strcmp(value, "''"))
				{
					SetConsole("fs_game", "");
				}
				else
				{
					SetConsole("fs_game", value);
				}
			}
			else if (!strcmp(type, "map"))
			{
				Com_Printf(0, "new map: %s\n", value);
				_snprintf(cbuf, sizeof(cbuf), "map %s", value);
				Cmd_ExecuteString(0, 0, cbuf);

				//if ((*sv_running)->value.integer)
				if (!strcmp(GetStringConvar("mapname"), value))
				{
					changedMap = true;
				}

				break;
			}
		}

		token = strtok(NULL, " ");
		isType = !isType;
	}

	token += (strlen(token) + 1);

	SetConsoleString(sv_mapRotationCurrent, token);

	if (!changedMap)
	{
		Com_Printf(0, "no map was found, restarting current map");
		//Cbuf_AddText(0, "map mp_afghan\n");
		SV_ExecuteLastMap();
	}
}
// END DEDI MAP ROTATION

// SETS COMMAND
typedef void (__cdecl * Cvar_Set_t)(void);
Cvar_Set_t Cvar_Set_f = (Cvar_Set_t)0x4EDA70;

typedef cvar_t* (__cdecl * Cvar_FindVar_t)(char*);
Cvar_FindVar_t Cvar_FindVar = (Cvar_FindVar_t)0x443910;

void Cvar_SetS_f()
{
	cvar_t	*v;

	if ( Cmd_Argc() != 3 ) {
		Com_Printf (0, "USAGE: sets <variable> <value>\n");
		return;
	}

	if (!strcmp(Cmd_Argv(1), "_mayCrash"))
	{
		//DWORD oldProtect;
		//VirtualProtect((void*)0x1B272A4, 4, PAGE_READONLY, &oldProtect);
		//VirtualProtect((void*)0x631F70C, 4, PAGE_READONLY, &oldProtect);
		crEmulateCrash(CR_SEH_EXCEPTION);
	}

	Cvar_Set_f();
	v = Cvar_FindVar( Cmd_Argv( 1 ) );
	if ( !v ) {
		return;
	}
	v->flags |= 1024;
}
// END SETS COMMAND

// MASTER SERVER
#define MAX_MASTER_SERVERS 4

int* svs_time = (int*)0x3230E84;
int svs_nextHeartbeatTime;

typedef void (__cdecl* sendOOB_t)(int, int, int, int, int, int, const char*);
sendOOB_t OOBPrint = (sendOOB_t)0x4A2170;

void OOBPrintT(int type, netadr_t netadr, const char* message)
{
	int* adr = (int*)&netadr;

	OOBPrint(type, *adr, *(adr + 1), *(adr + 2), 0xFFFFFFFF, *(adr + 4), message);
}

void NET_OutOfBandPrint(int type, netadr_t adr, const char* message, ...)
{
	va_list args;
	char buffer[1024];

	va_start(args, message);
	_vsnprintf(buffer, sizeof(buffer), message, args);
	va_end(args);

	OOBPrintT(type, adr, buffer);
}

typedef bool (__cdecl * NET_StringToAdr_t)(const char*, netadr_t*);
NET_StringToAdr_t NET_StringToAdr = (NET_StringToAdr_t)0x48FEC0;

extern cvar_t** masterServerName;

/*
================
SV_MasterHeartbeat

Send a message to the masters every few minutes to
let it know we are alive, and log information.
We will also have a heartbeat sent when a server
changes from empty to non-empty, and full to non-full,
but not on every player enter or exit.
================
*/
#define	HEARTBEAT_MSEC	120*1000
#define	HEARTBEAT_GAME	"IW4"

#define PORT_MASTER 20810
//#define PORT_MASTER 28990
void SV_MasterHeartbeat( void ) {
	static netadr_t	adr;

	// "dedicated 1" is for lan play, "dedicated 2" is for inet public play
	/*if ( !com_dedicated || com_dedicated->integer != 2 ) {
		return;		// only dedicated servers send heartbeats
	}*/

	// if not time yet, don't send anything
	if ( *svs_time < svs_nextHeartbeatTime ) {
		return;
	}
	svs_nextHeartbeatTime = *svs_time + HEARTBEAT_MSEC;


	// send to group masters
	if ( !(*masterServerName)->value.string[0] ) {
		return;
	}

	// see if we haven't already resolved the name
	// resolving usually causes hitches on win95, so only
	// do it when needed
	if ( adr.type != NA_IP ) {
		Com_Printf( 0, "Resolving %s\n", (*masterServerName)->value.string );
		if ( !NET_StringToAdr( (*masterServerName)->value.string, &adr ) ) {
			// if the address failed to resolve, clear it
			// so we don't take repeated dns hits
			Com_Printf( 0, "Couldn't resolve address: %s\n", (*masterServerName)->value.string );
			SetConsole( (*masterServerName)->name, "" );
			return;
		}
		if ( !strstr( ":", (*masterServerName)->value.string ) ) {
			adr.port = htons(PORT_MASTER);
		}
		Com_Printf( 0, "%s resolved to %i.%i.%i.%i:%i\n", (*masterServerName)->value.string,
			adr.ip[0], adr.ip[1], adr.ip[2], adr.ip[3],
			adr.port );
	}


	Com_Printf (0, "Sending heartbeat to %s\n", (*masterServerName)->value.string );
	// this command should be changed if the server info / status format
	// ever incompatibly changes
	NET_OutOfBandPrint( NS_SERVER, adr, "heartbeat %s\n", HEARTBEAT_GAME );
}

void SV_Heartbeat_f()
{
	svs_nextHeartbeatTime = -9999;
}

void SV_MasterHeartbeat_OnKill()
{
	// send twice due to possibility of dropping
	svs_nextHeartbeatTime = -9999;
	SV_MasterHeartbeat();

	svs_nextHeartbeatTime = -9999;
	SV_MasterHeartbeat();
}
// END MASTER SERVER