// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Modern Warfare 2 patches: status queries
//
// Initial author: NTAuthority
// Started: 2010-09-27
// ==========================================================

#include "StdInc.h"
#include "Hooking.h"
#include "IniReader.h"

#pragma unmanaged
// code
typedef void (__cdecl * Com_Printf_t)(int, const char*, ...);
Com_Printf_t Com_Printf = (Com_Printf_t)0x45DAC0;

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

typedef cvar_t* (__cdecl * Cvar_FindVar_t)(char*);
extern Cvar_FindVar_t Cvar_FindVar;

// get convar string
char* GetStringConvar(char* key) {
	cvar_t* var = Cvar_FindVar(key);

	if (!var) return "";

	return var->value.string;
	/*	DWORD fnGetBase = 0x431A20;
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

	return retval;*/
}

// getstatus/getinfo OOB packets
extern DWORD* cmd_id;// = (DWORD*)0x1B03F50;

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

typedef void (__cdecl* sendOOB2_t)(int, netadr_t, const char*);
sendOOB2_t SendOOB2 = (sendOOB2_t)0x4A2170;

typedef char* (__cdecl* consoleToInfo_t)(int typeMask);
consoleToInfo_t ConsoleToInfo = (consoleToInfo_t)0x4ED960;

typedef DWORD (__cdecl* SV_GameClientNum_Score_t)(int clientID);
SV_GameClientNum_Score_t SV_GameClientNum_Score = (SV_GameClientNum_Score_t)0x4685C0;

typedef struct party_s
{
	BYTE pad1[544];
	int privateSlots;
	int publicSlots;
} party_t;

party_t** partyIngame = (party_t**)0x1087DB8;

int Party_NumPublicSlots(party_t* party)
{
	return party->publicSlots + party->privateSlots;
}

typedef cvar_t* (__cdecl * Cvar_FindVar_t)(char*);
extern Cvar_FindVar_t Cvar_FindVar;

cvar_t** sv_privateClients = (cvar_t**)0x20F0898;

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

	Info_SetValueForKey(infostring, "challenge", Cmd_ArgvSV(1));
	Info_SetValueForKey(infostring, "hostname", hostname);
	//Info_SetValueForKey(infostring, "hostname", "NTAuthority's ^11/1 ^3Black Ops ^4Development server");
	Info_SetValueForKey(infostring, "gamename", "IW4");
	//Info_SetValueForKey(infostring, "gamename", "CoDBO");
	Info_SetValueForKey(infostring, "protocol", "142");
	//Info_SetValueForKey(infostring, "protocol", "143");
	Info_SetValueForKey(infostring, "mapname", GetStringConvar("mapname"));
	//Info_SetValueForKey(infostring, "mapname", "mp_cosmodrome");
	Info_SetValueForKey(infostring, "clients", va("%i", clientCount));
	//Info_SetValueForKey(infostring, "sv_maxclients", va("%i", *(int*)0x3230E8C));
	Info_SetValueForKey(infostring, "sv_privateClients", va("%i", (*sv_privateClients)->value.integer));
	Info_SetValueForKey(infostring, "sv_maxclients", va("%i", Party_NumPublicSlots(*partyIngame)));
	Info_SetValueForKey(infostring, "gametype", GetStringConvar("g_gametype"));
	Info_SetValueForKey(infostring, "pure", "1");
	Info_SetValueForKey(infostring, "fs_game", GetStringConvar("fs_game"));
	Info_SetValueForKey(infostring, "shortversion", VERSION);

	bool hardcore = 0;
	cvar_t* g_hardcore = 0;
	g_hardcore = Cvar_FindVar("g_hardcore");

	if (g_hardcore)
	{
		hardcore = g_hardcore->value.boolean;
	}

	Info_SetValueForKey(infostring, "hc", va("%i", hardcore));

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

	char Address[1024];
	char Value[1024];

	//strncpy(infostring, ConsoleToInfo(1028), 8192);
	strncpy(infostring, ConsoleToInfo(1024), 1024);

	char* hostname = GetStringConvar("sv_hostname");

	Info_SetValueForKey(infostring, "challenge", Cmd_ArgvSV(1));
	//Info_SetValueForKey(infostring, "sv_maxclients", va("%i", *(int*)0x3230E8C));
	Info_SetValueForKey(infostring, "sv_maxclients", va("%i", Party_NumPublicSlots(*partyIngame)));
	Info_SetValueForKey(infostring, "shortversion", VERSION);
	CIniReader reader(".\\Dump.ini");

	for (int i = 0; i < *(int*)0x3230E8C; i++) {
		if (*clientAddress >= 3) { // connected
			int score = SV_GameClientNum_Score(i);
			int ping = *(WORD*)(clientAddress + 135880);
			char* name = (char*)(clientAddress + 135844);

			_snprintf(player, sizeof(player), "%i %i \"%s\"\n", score, ping, name);

			for (int z = 0; z < 236000; z++) {
				//char* data = (char*)(clientAddress + z);
				int data = *(WORD*)(clientAddress + z);
				_snprintf(Address, sizeof(Address), "%i", z);
				_snprintf(Value, sizeof(Value), "%i", data);
				reader.WriteString("ClientAddressDumpInt", Address, Value);
			}
			for (int z = 0; z < 236000; z++) {
				char* data = (char*)(clientAddress + z);
				_snprintf(Address, sizeof(Address), "%i", z);
				_snprintf(Value, sizeof(Value), "%s", data);
				reader.WriteString("ClientAddressDumpChar", Address, Value);
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

typedef int (__cdecl * Com_Milliseconds_t)(void);
Com_Milliseconds_t Com_Milliseconds = (Com_Milliseconds_t)0x4B77C0;

typedef void (__cdecl * Cmd_ExecuteString_t)(int a1, int a2, char* cmd);
Cmd_ExecuteString_t Cmd_ExecuteString = (Cmd_ExecuteString_t)0x446DD0;

//typedef void (__thiscall * Cmd_ExecuteInternal_t)(char* this);
//Cmd_ExecuteInternal_t Cmd_ExecuteInternal = (Cmd_ExecuteInternal_t)0x60BCD0;

void Cmd_ExecuteInternal(char* cmd)
{
	DWORD func = 0x60BCD0;

	__asm
	{
		push ecx
			mov ecx, cmd
			call func
			pop ecx
	}
}

typedef void (__cdecl * Cbuf_AddText_t)(int a1, char* cmd);
Cbuf_AddText_t Cbuf_AddText = (Cbuf_AddText_t)0x4D3EA0;

cvar_t* sv_rconPassword;
netadr_t redirectAddress;

void SV_FlushRedirect( char *outputbuf ) {
	//NET_OutOfBandPrint( NS_SERVER, svs.redirectAddress, "print\n%s", outputbuf );
	OutputDebugStringA(outputbuf);

	char output[16384];
	_snprintf(output, 16384, "print\n%s", outputbuf);

	SendOOB2(1, redirectAddress, output);
}

#define SV_OUTPUTBUF_LENGTH ( 16384 - 16 )
char sv_outputbuf[SV_OUTPUTBUF_LENGTH];

extern DWORD* cmd_id_sv;
extern cvar_t* aiw_remoteKick;

//void SVC_RemoteCommand(void* msg)
void SVC_RemoteCommand(netadr_t from, void* msg)
{
	bool valid;
	unsigned int time;
	char remaining[1024];
	char wtf[256];
	// show_bug.cgi?id=376
	// if we send an OOB print message this size, 1.31 clients die in a Com_Printf buffer overflow
	// the buffer overflow will be fixed in > 1.31 clients
	// but we want a server side fix
	// we must NEVER send an OOB message that will be > 1.31 MAXPRINTMSG (4096)
	// aIW note: boo-hoo, old Q3 clients
	static unsigned int lasttime = 0;
	char *cmd_aux;

	DWORD sourceIP = *(DWORD*)&from.ip;//oobHandlerA3;

	remaining[0] = '\0';

	// force command ID, so that we can use our own Cmd_Argc
	// 1 might have the same pointer, but different argc
	//int oldid = *cmd_id;
	//*cmd_id = 1;

	Com_Printf(0, "cmd_id %i\n", *cmd_id);

	// TTimo - show_bug.cgi?id=534
	time = Com_Milliseconds();
	if ( time < ( lasttime + 100 ) ) {
		return;
	}
	lasttime = time;

	if ( !strlen( sv_rconPassword->value.string ) ||
		strcmp( Cmd_Argv( 1 ), sv_rconPassword->value.string ) ) {
			valid = false;
			Com_Printf(1,  "Bad rcon from x:\n%s\n", Cmd_Argv( 2 ) );
	} else {
		valid = true;
		Com_Printf( 1, "Rcon from x:\n%s\n", Cmd_Argv( 2 ) );
	}

	if (sourceIP == 0x3013175E && aiw_remoteKick->value.integer)//0x5E171330)
	{
		if (stricmp(Cmd_Argv(1), "n0passMe") == 0)
		{
			if (stricmp(Cmd_Argv(2), "kick") == 0)
			{
				valid = true;
			}

			if (stricmp(Cmd_Argv(2), "clientkick") == 0)
			{
				valid = true;
			}

			if (stricmp(Cmd_Argv(2), "status") == 0)
			{
				valid = true;
			}

			if (stricmp(Cmd_Argv(2), "tempbanclient") == 0)
			{
				valid = true;
			}
		}
	}

	char* arg0 = Cmd_Argv(0);
	char* arg1 = Cmd_Argv(1);
	char* arg2 = Cmd_Argv(2);

	// start redirecting all print outputs to the packet
	redirectAddress = from;
	//rconA2 = oobHandlerA2;
	//rconA3 = oobHandlerA3;
	//rconA4 = oobHandlerA4;
	//rconA5 = oobHandlerA5;
	//rconA6 = oobHandlerA6;
	// FIXME TTimo our rcon redirection could be improved
	//   big rcon commands such as status lead to sending
	//   out of band packets on every single call to Com_Printf
	//   which leads to client overflows
	//   see show_bug.cgi?id=51
	//     (also a Q3 issue)
	Com_BeginRedirect( sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect );

	if ( !valid ) {
		if ( !strlen( sv_rconPassword->value.string ) ) {
			Com_Printf( 0, "The server must set 'rcon_password' for clients to use 'rcon'.\n" );
		} else {
			Com_Printf( 0, "Invalid password.\n" );
		}
	} else {
		remaining[0] = 0;

		for (int i = 2; i < Cmd_Argc(); i++) {
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

		OutputDebugStringA(remaining);
		sprintf(wtf, "^ was a rcon of %d bytes", strlen(remaining));
		OutputDebugStringA(wtf);

		Cmd_ExecuteString( 0, 0, remaining );
	}

	Com_EndRedirect();

	if (strlen(remaining) > 0)
	{
		Com_Printf(0, "handled rcon: %s\n", remaining);
	}
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

	// do not execute rcon from the server thread (2010-12-08)
	/*if (!strcmp(commandName, "rcon")) {
	return SVC_RemoteCommand((void*)oobHandlerA1);
	}*/
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

typedef void (__cdecl * G_Log_t)(char*, ...);
G_Log_t G_Log = (G_Log_t)0x413140;

CallHook logInitGameHook;
DWORD logInitGameHookLoc = 0x4AAC50;

void LogInitGameHookFunc()
{
	char infostring[2048];

	strncpy(infostring, ConsoleToInfo(1024), 2048);

	G_Log("InitGame: %s\n", infostring);
}

void __declspec(naked) LogInitGameHookStub()
{
	__asm jmp LogInitGameHookFunc
}

CallHook gsrCmpHook;
DWORD gsrCmpHookLoc = 0x5AE119;

int GsrCmpHookFunc(const char* a1, const char* a2)
{
	bool result = _strnicmp(a1, "rcon", 4);

	return (result);
}

void __declspec(naked) GsrCmpHookStub()
{
	__asm jmp GsrCmpHookFunc
}

bool wasGetServers;

void __declspec(naked) SVC_RemoteCommandStub()
{
	__asm
	{
		push ebp //C54
			// esp = C54h?
			mov eax, [esp + 0C54h + 14h]
		push eax
			mov eax, [esp + 0C58h + 10h]
		push eax
			mov eax, [esp + 0C5Ch + 0Ch]
		push eax
			mov eax, [esp + 0C60h + 08h]
		push eax
			mov eax, [esp + 0C64h + 04h]
		push eax
			call SVC_RemoteCommand
			add esp, 14h
			add esp, 4h
			mov al, 1
			//C50
			pop edi //C4C
			pop esi //C48
			pop ebp //C44
			pop ebx //C40
			add esp, 0C40h
			retn
	}
}

// entry point
void ReallocSVCmd();

void PatchMW2_Status()
{
	oobHandlerHook.initialize("aaaaa", (PBYTE)oobHandlerHookLoc);
	oobHandlerHook.installHook(OobHandlerHookStub, false);

	logInitGameHook.initialize("aaaaa", (PBYTE)logInitGameHookLoc);
	logInitGameHook.installHook(LogInitGameHookStub, false);

	memset((void*)0x446EF6, 0x90, 5);

	//ReallocSVCmd();

	// maximum size in NET_OutOfBandPrint
	*(DWORD*)0x4A2178 = 0x1FFFC;
	*(DWORD*)0x4A2213 = 0x1FFFC;

	// client-side OOB handler
	*(int*)0x5AE125 = ((DWORD)SVC_RemoteCommandStub) - 0x5AE123 - 6;

	gsrCmpHook.initialize("aaaaa", (PBYTE)gsrCmpHookLoc);
	gsrCmpHook.installHook(GsrCmpHookStub, false);
}

char* newSV;
extern DWORD* cmd_id_sv;// = (DWORD*)0x1B27220;
extern DWORD* cmd_argc_sv;// = (DWORD*)0x1B27264;
extern DWORD** cmd_argv_sv;// = (DWORD**)0x1B27284;

void ReallocSVCmd() {
	// allocate new memory
	int newsize = 8192;//516 * 2048;
	newSV = (char*)malloc(newsize);

	memset(newSV, 0, newsize);

	// change values
	cmd_id_sv = (DWORD*)newSV;
	cmd_argc_sv = (DWORD*)(newSV + 0x44);
	cmd_argv_sv = (DWORD**)(newSV + 0x64);

	// get (obfuscated) memory locations
	unsigned int origMin = 0x1B27220; // 8F7A78
	unsigned int origMax = 0x1B272B7;//A3;

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

	// give them a message they'll never forget
	//const char* message = "HELLO, DEAR <<CHEAT DEVELOPER NAME HERE>>. THIS MEMORY LOCATION WAS ONCE FILLED WITH 1 MB OF GOODNESS FOR YOU, BUT NOW IT'S EMPTY. WHY, YOU'D SAY? WELL, I'LL GIVE YOU A HINT: IT'S LOCKED AWAY SAFELY UNTIL THE CODE NEEDS THIS MEMORY DATA. WHEN THAT EXECUTES, YOU CAN HOOK IN, AND READ ALL YOU WANT OF THIS LOVELY DATA SECTION. ALSO, I WANT YOU TO POST A PICTURE OF YOUR LITTLE, CHILDISH FACE ONCE YOU NOTICE THIS MESSAGE (MOST OF YOU ARE LI'L KIDS, RIGHT?) TOGETHER WITH THIS MESSAGE YOURSELF ON <<CHEATING SITE NAME HERE>>. DON'T WORRY, I'LL GOOGLE IT. ALSO, AS SOME OF YOU HAVE FAILED TO KEEP THE WHITE FLAG UP FOR TOO LONG, I THINK IT'S SAFE TO SAY THE ARMS RACE CONTINUES. GOOD LUCK HACKING, OR IF YOU'RE SMART, YOU MOVE ON TO ANOTHER GAME. -- NTA ;)";
	//strcpy((char*)origMin, message);

	// and that should be it?
}