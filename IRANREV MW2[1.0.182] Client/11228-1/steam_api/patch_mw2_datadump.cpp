#include "stdafx.h"
#include "Hooking.h"

using namespace System;
using namespace System::Net;
using namespace System::Threading;

void Com_BeginRedirect( char *buffer, int buffersize, void ( *flush )( char * ) );
void Com_EndRedirect( void );
unsigned int GetPlayerSteamID();

typedef void (__cdecl * Cmd_ExecuteString_t)(int a1, int a2, char* cmd);
extern Cmd_ExecuteString_t Cmd_ExecuteString;

bool isUploading = false;

void DoUploadData(char* data)
{
	try
	{
		String^ steamID = GetPlayerSteamID().ToString("X8");
		String^ text = gcnew String(data);

		WebClient^ wc = gcnew WebClient();
		//wc->UploadString("http://alteriw.net/stats/store/" + steamID, text);
		wc->UploadString("http://localhost/aiwid/stats/store.php?id=" + steamID, text);

		isUploading = false;
	}
	catch (Exception^ ex)
	{
		OutputDebugStringA("bo");
	}
}

void UploadStatData(Object^ state)
{
	if (isUploading)
	{
		return;
	}

	isUploading = true;

	static char localBuf[512 * 1024];
	Com_BeginRedirect(localBuf, sizeof(localBuf), DoUploadData);
	Cmd_ExecuteString(0, 0, "dumpPlayerData");
	Com_EndRedirect();
}

void DumpStatsOnWrite(void* buffer, int bufferSize)
{
	if (isUploading)
	{
		return;
	}

	ThreadPool::QueueUserWorkItem(gcnew WaitCallback(&UploadStatData));
}

#pragma unmanaged
static char *rd_buffer;
static int rd_buffersize;
static void ( *rd_flush )( char *buffer );

void Com_BeginRedirect( char *buffer, int buffersize, void ( *flush )( char *) ) {
	if ( !buffer || !buffersize || !flush ) {
		return;
	}
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

void Com_EndRedirect( void ) {
	if ( rd_flush ) {
		rd_flush( rd_buffer );
	}

	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}

// com_printf to com_print
CallHook printfHook;
DWORD printfHookLoc = 0x45DB00;

// 'local' vars
int level;
const char* msg;
int wtf;

void __declspec(naked) PrintfHookStub()
{
	__asm push eax
	__asm mov eax, [esp + 8h]
	__asm mov level, eax
	__asm mov eax, [esp + 0Ch]
	__asm mov msg, eax
	__asm mov eax, [esp + 10h]
	__asm mov wtf, eax
	__asm pop eax

	if (rd_buffer)
	{
		// quick fix for some stack fuckups here
		__asm pushad
		if ( ( strlen( msg ) + strlen( rd_buffer ) ) > ( rd_buffersize - 1 ) ) {
			rd_flush( rd_buffer );
			*rd_buffer = 0;
		}
		strncat( rd_buffer, msg, rd_buffersize );

		__asm popad
		__asm retn
	}

	__asm jmp printfHook.pOriginal
}

void PatchMW2_Redirect()
{
	printfHook.initialize("insignia is a cake", (PBYTE)printfHookLoc);
	printfHook.installHook(PrintfHookStub, false);
}