// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches
//
// Initial author: NTAuthority
// Started: 2010-09-27
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"
#include "detours.h"
#include <winsock.h>
#include <CrashRpt.h>

// very unsafe code follows
#pragma unmanaged

void PatchMW2_182();

void PatchMW2()
{
	// check version
	// TODO before full release: make version-independent patches
	if (!strcmp((char*)0x73C4C0, "182")) { // 1.0.182 version-specific address
		PatchMW2_182();
		return;
	}

	TerminateProcess(GetCurrentProcess(), 0);
}

CallHook winMainInitHook;
DWORD winMainInitHookLoc = 0x4AEDB0;

void InstallCrashRpt();

void __declspec(naked) WinMainInitHookStub() {
	InstallCrashRpt();

	__asm {
		jmp winMainInitHook.pOriginal
	}
}

void PatchMW2_FFHash();
void PatchMW2_Modding();
void PatchMW2_Dedicated();
void PatchMW2_Status();
void PatchMW2_Redirect();
void PatchMW2_Servers();
void PatchMW2_Load();
//void PatchMW2_MatchData();

void PatchMW2_182()
{
	// protocol version (workaround for hacks)
	//*(int*)0x41FB31 = 0x90; // was 8E
	*(int*)0x41FB31 = 5389; // was 8E

	// protocol command
	*(int*)0x4BB9D9 = 0x90; // was 8E
	*(int*)0x4BB9DE = 0x90; // was 8E
	*(int*)0x4BB9E3 = 0x90; // was 8E
	
	//*(int*)0x4BB9D9 = 0x90; // was 8E
	//*(int*)0x4BB9DE = 0x90; // was 8E
	//*(int*)0x4BB9E3 = 0x90; // was 8E

	// un-neuter Com_ParseCommandLine to allow non-connect_lobby
	*(BYTE*)0x4AECA4 = 0xEB;

	// remove system pre-init stuff (improper quit, disk full)
	*(BYTE*)0x4A8E90 = 0xC3;

	// logfile minimum at 0
	*(BYTE*)0x60D515 = 0;

	// remove STEAMSTART checking for DRM IPC
	memset((void*)0x470A25, 0x90, 5);
	*(BYTE*)0x470A2C = 0xEB;

	// master server (server.alteriw.net)
	strcpy((char*)0x6E394C, "server.iw4.ir");//was "server.alteriw.net"

	// internal version is 99, most servers should accept it
	*(int*)0x4024B1 = 99;

	// patch web1 server
	const char* webName = "http://web1.pc.iw4.iwnet.infinityward.com:13000/pc/";

	*(DWORD*)0x4CD220 = (DWORD)webName;
	*(DWORD*)0x4CD23F = (DWORD)webName;

	// winmain
	winMainInitHook.initialize("antic", (PBYTE)winMainInitHookLoc);
	winMainInitHook.installHook(WinMainInitHookStub, false);

	// more detailed patches
	PatchMW2_FFHash();
	PatchMW2_Modding();
	PatchMW2_Dedicated();
	PatchMW2_Status();
	PatchMW2_Redirect();
	PatchMW2_Servers();
	PatchMW2_Load();
	//PatchMW2_MatchData();
}

// smaller patches go below

// patches fastfile integrity checking
void PatchMW2_FFHash()
{
	// basic checks (hash jumps, both normal and playlist)
	*(WORD*)0x5BD502 = 0x9090;
	*(WORD*)0x5BD4BC = 0x9090;

	*(WORD*)0x5BDD02 = 0x9090;
	*(WORD*)0x5BDCBC = 0x9090;

	*(WORD*)0x5BD343 = 0x9090;
	*(WORD*)0x5BD354 = 0x9090;

	*(WORD*)0x5BDB43 = 0x9090;
	*(WORD*)0x5BDB54 = 0x9090;

	// some other, unknown, check
	*(BYTE*)0x5BD4B2 = 0xB8;
	*(DWORD*)0x5BD4B3 = 1;

	*(BYTE*)0x5BD4F8 = 0xB8;
	*(DWORD*)0x5BD4F9 = 1;
}

CallHook execIsFSHook;
DWORD execIsFSHookLoc = 0x60C00D; // COOD! nice :)

#pragma managed
using namespace System;
using namespace System::IO;

bool ExecIsFSHookFunc(const char* execFilename, const char* dummyMatch) { // dummyMatch isn't used by us
	// quick fix for config_mp.cfg (UAC virtualization?) issues
	// hardcode config_mp.cfg as the stock game does, as GetFileAttributesA seems to ignore UAC virtualization
	// enable if next code causes even more issues
	if (!strcmp(execFilename, "autoexec.cfg")) {
		return false;
	}

	// possible real fix would go a bit like this, would be similar to GSH chdir issues
	// don't trust current directory, generate application dir
	char appPath[MAX_PATH];
	char appDir[MAX_PATH];

	memset(appDir, 0, sizeof(appDir));

	GetModuleFileNameA(NULL, appPath, sizeof(appPath) - 1);
	strncpy(appDir, appPath, strrchr(appPath, '\\') - appPath);
	appDir[strlen(appDir)] = '\0';

	// before fix was just players\[execFilename]
	char filename[MAX_PATH];
	_snprintf(filename, MAX_PATH, "%s\\%s\\%s", appDir, "players", execFilename);

	String^ sFilename = gcnew String(filename);

	//crEmulateCrash(CR_SEH_EXCEPTION);

	if (!File::Exists("main\\iw_25.iwd"))
	{
		Windows::Forms::MessageBox::Show("It seems main\\iw_25.iwd does not exist. Are you sure you installed all aIW updates before using the dedicated server?");
		TerminateProcess(GetCurrentProcess(), 0);
	}

	if (!File::Exists(sFilename)) {
		_snprintf(filename, MAX_PATH, "%s\\%s\\%s", appDir, "main", execFilename);

		sFilename = gcnew String(filename);
		if (!File::Exists(sFilename)) {
			return true; // not FS, try fastfile
		}
	}

	return false;
}

#pragma unmanaged

void __declspec(naked) ExecIsFSHookStub() {
	__asm jmp ExecIsFSHookFunc
}

DWORD pureShouldBeZero = 0;

// patches fs_game/IWD script support
void PatchMW2_Modding()
{
	// remove limit on IWD file loading
	memset((void*)0x643B94, 0x90, 6);

	// remove convar write protection
	*(BYTE*)0x647DD4 = 0xEB;

	// kill most of pure (unneeded in 159, 180+ messed it up)
	memset((void*)0x45513D, 0x90, 5);
	memset((void*)0x45515B, 0x90, 5);
	memset((void*)0x45516C, 0x90, 5);

	memset((void*)0x45518E, 0x90, 5);
	memset((void*)0x45519F, 0x90, 5);

	*(BYTE*)0x449089 = 0xEB;

	// other IWD things (pure?)
	*(BYTE*)0x4C5E7B = 0xEB;
	*(BYTE*)0x465107 = 0xEB;

	// remove 'impure stats' checking
	*(BYTE*)0x501A30 = 0x33;
	*(BYTE*)0x501A31 = 0xC0;
	*(DWORD*)0x501A32 = 0xC3909090;

	// remove fs_game profiles
	*(WORD*)0x48D9C4 = 0x9090;

	// fs_game crash fix removing some calls
	*(BYTE*)0x481F1D = 0xEB;

	// remove fs_game check for moddable rawfiles - allows non-fs_game to modify rawfiles
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
	// CLARIFY 2010-09-27: we don't have textual weapon files, as such we should load them from fastfile as usual
	*(BYTE*)0x43A3ED = 0xEB;

	// more pure removal
	*(DWORD*)0x79BAA4 = (DWORD)&pureShouldBeZero;

	// exec fixing
	execIsFSHook.initialize("aaaaa", (PBYTE)execIsFSHookLoc);
	execIsFSHook.installHook(ExecIsFSHookStub, false);

	// exec whitelist removal (YAYFINITY WARD)
	memset((void*)0x60BD95, 0x90, 5);
	*(WORD*)0x60BD9C = 0x9090;
}

// installer for crashrpt
BOOL WINAPI CrashCallback(LPVOID lpvState)
{
	//crAddFile2A("iw4mp.cfg", "iw4mp.cfg", "Indigo configuration file", CR_AF_MAKE_FILE_COPY);
	crAddFile2A("main\\console_mp.log", "console_mp.log", "Game console log file", CR_AF_MAKE_FILE_COPY);
	//crAddFile2A("alteriwnet.ini", "alteriwnet.ini", "alterIWnet configuration", CR_AF_MAKE_FILE_COPY);
	//crAddFile2A("caches.xml", "caches.xml", "Game file versions", CR_AF_MAKE_FILE_COPY);
	//crAddScreenshot(CR_AS_VIRTUAL_SCREEN);

	return TRUE;
}

void InstallCrashRpt() {
	CR_INSTALL_INFOA crInstallInfo;
	memset(&crInstallInfo, 0, sizeof(CR_INSTALL_INFOA));
	crInstallInfo.cb = sizeof(CR_INSTALL_INFOA);  
	crInstallInfo.pszAppName = "alterIWnet server";
	crInstallInfo.pszAppVersion = VERSION;
	crInstallInfo.pfnCrashCallback = CrashCallback;
	crInstallInfo.uPriorities[CR_HTTP] = 99;
	crInstallInfo.uPriorities[CR_SMTP] = -1;
	crInstallInfo.uPriorities[CR_SMAPI] = -1;
	crInstallInfo.pszUrl = "http://alteriw.net/dedicate/crashrpt.php";
	crInstallInfo.dwFlags = 0;
	crInstallInfo.dwFlags |= CR_INST_NO_GUI;
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

		TCHAR szErrorMsg[256];
		crGetLastErrorMsg(szErrorMsg, 256);

		OutputDebugString(szErrorMsg);
	}
}


// gethostbyname hook
cvar_t** masterServerName = (cvar_t**)0x1B308C8;

typedef hostent* (WINAPI *gethostbyname_t)(const char* name);
gethostbyname_t realgethostbyname = (gethostbyname_t)gethostbyname;

char* serverName = NULL;
char* webName = NULL;

unsigned int oneAtATimeHash(char* inpStr)
{
	unsigned int value = 0,temp = 0;
	for(size_t i=0;i<strlen(inpStr);i++)
	{
		char ctext = tolower(inpStr[i]);
		temp = ctext;
		temp += value;
		value = temp << 10;
		temp += value;
		value = temp >> 6;
		value ^= temp;
	}
	temp = value << 3;
	temp += value;
	unsigned int temp2 = temp >> 11;
	temp = temp2 ^ temp;
	temp2 = temp << 15;
	value = temp2 + temp;
	if(value < 2) value += 2;
	return value;
}

hostent* WINAPI custom_gethostbyname(const char* name) {
	// if the name is IWNet's stuff...
	unsigned int ip1 = oneAtATimeHash("ip1.pc.iw4.iwnet.infinityward.com");
	unsigned int log1 = oneAtATimeHash("log1.pc.iw4.iwnet.infinityward.com");
	unsigned int match1 = oneAtATimeHash("match1.pc.iw4.iwnet.infinityward.com");
	unsigned int web1 = oneAtATimeHash("web1.pc.iw4.iwnet.infinityward.com");
	unsigned int blob1 = oneAtATimeHash("blob1.pc.iw4.iwnet.infinityward.com");

	unsigned int current = oneAtATimeHash((char*)name);
	char* hostname = (char*)name;

	if (current == log1 || current == match1 || current == blob1) {
		hostname = (*masterServerName)->value.string;
	}

	if (current == ip1) {
		hostname = (*masterServerName)->value.string;
	}

	if (current == web1) {
		hostname = (*masterServerName)->value.string;
	}

	return realgethostbyname(hostname);
}

void PatchMW2_Servers()
{
	// gethostbyname
	PBYTE offset = (PBYTE)GetProcAddress(GetModuleHandleA("ws2_32.dll"),"gethostbyname");
	realgethostbyname = (gethostbyname_t)DetourFunction(offset, (PBYTE)&custom_gethostbyname);
}

// COMMON FUNCTIONS

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

DWORD* cmd_id_sv = (DWORD*)0x1B27220;
DWORD* cmd_argc_sv = (DWORD*)0x1B27264;
DWORD** cmd_argv_sv = (DWORD**)0x1B27284;

/*
============
Cmd_Argc
============
*/
int		Cmd_ArgcSV( void ) {
	return cmd_argc_sv[*cmd_id_sv];
}

/*
============
Cmd_Argv
============
*/
char	*Cmd_ArgvSV( int arg ) {
	if ( (unsigned)arg >= cmd_argc_sv[*cmd_id_sv] ) {
		return "";
	}
	return (char*)(cmd_argv_sv[*cmd_id_sv][arg]);	
}