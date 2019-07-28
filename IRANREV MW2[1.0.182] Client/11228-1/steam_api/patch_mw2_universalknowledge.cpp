#include "stdafx.h"
#include "Hooking.h"

void DumpStatsOnWrite(void* buffer, int bufferSize);

#pragma unmanaged

const char* wtfuk = "Universal knowledge module (patch_mw2_universalknowledge.cpp). Allowing sharing of the world's favorite 8192 bytes buffer (on most systems containing a value like 2516000) between incompatible user IDs.";

StompHook writeStatsHook;
DWORD writeStatsHookLoc = 0x682600;

const char* fileID;
bool useSteamCloud;
unsigned int bufferSize;

typedef bool (__cdecl * FS_WriteFile_t)(char* filename, char* folder, void* buffer, int size);
FS_WriteFile_t FS_WriteFile = (FS_WriteFile_t)0x4314F0;

bool WriteStatsFunc(int remote, void* buffer)
{
	// filename buffer
	char filename[256];

	// check buffer size to be valid, though it doesn't matter if we don't do TEA
	if (bufferSize > 8192 || bufferSize & 3) return false; // & 3 is for 4-byte alignment

	// ask about dumping stats
	DumpStatsOnWrite(buffer, bufferSize);

	// sprintf a filename
	_snprintf(filename, sizeof(filename), "%s_aIW.stat", fileID);

	// and write the file
	return FS_WriteFile(filename, "players", buffer, bufferSize);
}

void __declspec(naked) WriteStatsHookStub()
{
	__asm
	{
		// prepare weird optimized parameters
		mov fileID, ecx
		mov useSteamCloud, dl
		mov bufferSize, eax

		// and jump to our normal cdecl function
		jmp WriteStatsFunc
	}
}

StompHook readStatsHook;
DWORD readStatsHookLoc = 0x682820;
DWORD oldReadStats = 0x68282A;

void __declspec(naked) OldReadStats()
{
	__asm
	{
		mov eax, [esp + 10h]
		sub esp, 118h

		jmp oldReadStats
	}
}

int ReadStatsLegacy(int useSteamCloud, char* fileID, void* buffer, int isize)
{
	__asm
	{
		push isize
		push buffer
		push fileID
		push useSteamCloud

		call OldReadStats

		add esp, 10h
	}
}

typedef int (__cdecl * FS_ReadFile_t)(const char* qpath, void** buffer);
extern FS_ReadFile_t FS_ReadFile;

void SetSteamIDLegacy(bool legacy);

int ReadStatsFunc(int useSteamCloud, char* fileID, void* buffer, int size)
{
	// filename buffer
	char filename[256];

	// check buffer size to be valid, though it doesn't matter if we don't do TEA
	if (bufferSize > 8192 || bufferSize & 3) return 2; // & 3 is for 4-byte alignment

	// sprintf a filename
	_snprintf(filename, sizeof(filename), "%s_aIW.stat", fileID);

	// read file length/existence
	int length = FS_ReadFile(filename, 0);

	if (length == size)
	{
		void* fileData = NULL;
		FS_ReadFile(filename, &fileData);

		if (fileData)
		{
			memcpy(buffer, fileData, size);

			return 0;
		}
	}
	else if (length < 0)
	{
		// we don't have file
		// try old file
		// if won't work
		// user have no stats
		// and want play

		// mark SteamID as requiring 'legacy' code (with all its added bugginess :/ )
		SetSteamIDLegacy(true);

		// call old function (yes, I said *call*, not *jump*)
		int result = ReadStatsLegacy(useSteamCloud, fileID, buffer, size);

		// reset legacy flag and return result
		SetSteamIDLegacy(false);

		return result;
	}

	return 1;
}

void __declspec(naked) ReadStatsHookStub()
{
	__asm
	{
		// jump to cdecl func
		jmp ReadStatsFunc
	}
}

typedef struct statFile_s
{
	char* buffer;
	int number;
} statFile_t;

typedef int (__cdecl * GetPersistentData_t)(char** name, int type, statFile_t* stats);
GetPersistentData_t GetPersistentData = (GetPersistentData_t)0x4A1520;

CallHook lockStatsHook;
DWORD lockStatsHookLoc = 0x60CF3A;//0x4ADDC0;
DWORD lockStatsHookRet = 0x4ADDC6;

char* oldStats;
char* newStats;

bool LockStatsFunc()
{
	// made names non-obvious to prevent tampering
	char* prestige = (char*)0x719360;//"prestige";
	char* experience = (char*)0x6DC9A0;//"experience";

	statFile_t olds;
	statFile_t news;

	olds.buffer = oldStats + 4;
	olds.number = 8188;
	
	news.buffer = newStats + 4;
	news.number = 8188;

	int oldxp = GetPersistentData(&experience, 1, &olds);
	int newxp = GetPersistentData(&experience, 1, &news);

	int difference = newxp - oldxp;

	if (difference > 200000)
	{
		return true;
	}

	int oldpr = GetPersistentData(&prestige, 1, &olds);
	int newpr = GetPersistentData(&prestige, 1, &news);

	difference = newpr - oldpr;

	if (difference > 1)
	{
		return true;
	}

	return false;
}

void __declspec(naked) LockStatsHookStub()
{
	__asm
	{
		mov eax, [esp + 4h]
		mov oldStats, eax

		mov eax, [esp + 8h]
		mov newStats, eax

		call LockStatsFunc

		test eax, eax
		jz doRegression
		retn

doRegression:
		jmp lockStatsHook.pOriginal
	}
}

void UKnowInit()
{
	OutputDebugStringA("UKnowInit()");
	OutputDebugStringA(wtfuk);

	writeStatsHook.initialize("aaaaa", 5, (PBYTE)writeStatsHookLoc);
	writeStatsHook.installHook(WriteStatsHookStub, true, false);

	readStatsHook.initialize("aaaaa", 10, (PBYTE)readStatsHookLoc);
	readStatsHook.installHook(ReadStatsHookStub, true, false);

	lockStatsHook.initialize("aaaaa", (PBYTE)lockStatsHookLoc);
	lockStatsHook.installHook(LockStatsHookStub, false);
}

// RSA CODE
#pragma managed
using namespace System;
using namespace System::Net;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Cryptography;

ref class RSAProc
{
private:
	static RSACryptoServiceProvider^ _rsa;

	static bool _usingRSA;
public:
	static void Initialize(String^ server)
	{
		try
		{
			WebClient^ wc = gcnew WebClient();
			String^ url = String::Format("http://{0}:13000/key_public.xml", server);
			String^ result = wc->DownloadString(url);

			if (result->Contains("RSAKeyValue"))
			{
				_rsa = gcnew RSACryptoServiceProvider(2048);
				_rsa->FromXmlString(result);

				_usingRSA = true;
			}
			else
			{
				_usingRSA = false;
			}
		}
		catch (Exception^ e)
		{
			_usingRSA = false;
		}
	}
	
	static int Encrypt(const char* buf, int len, const char** newbuf)
	{
		if (!_usingRSA || len > 200)
		{
			*newbuf = buf;
			return len;
		}

		cli::array<Byte>^ bytes = gcnew cli::array<Byte>(len);
		IntPtr bufPtr((void*)buf);
		Marshal::Copy(bufPtr, bytes, 0, len);

		cli::array<Byte>^ encBytes = _rsa->Encrypt(bytes, true);
		cli::array<Byte>^ encBytesStart = gcnew cli::array<Byte>(encBytes->Length + 1);

		encBytesStart[0] = 0xFE;
		Array::Copy(encBytes, 0, encBytesStart, 1, encBytes->Length);

		pin_ptr<Byte> encPtr = &encBytesStart[0];
		*newbuf = (const char*)encPtr;

		return encBytesStart->Length;
	}
};

void RSAProc_Initialize(String^ server)
{
	RSAProc::Initialize(server);
}

int RSAProc_Encrypt(const char* buf, int len, const char** newbuf)
{
	return RSAProc::Encrypt(buf, len, newbuf);
}

#pragma unmanaged

int __stdcall sendto_rsa(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen)
{
	const char* newbuf;
	int newlen = RSAProc_Encrypt(buf, len, &newbuf);

	sendto(s, newbuf, newlen, flags, to, tolen);

	return len;
}

CallHook lspSendHook;
DWORD lspSendHookLoc = 0x4356D1;

void __declspec(naked) sendto_hook()
{
	__asm jmp sendto_rsa
}

CallHook aliasHook;
DWORD aliasHookLoc = 0x430FA4;

void* log1_ip = (void*)0x67235E4;
short* log1_port = (short*)0x67235EC;

typedef void (__cdecl * SendLSP_t)(const char* buf, int len, void* target);
SendLSP_t SendLSP = (SendLSP_t)0x4356A0;

void SetSteamIDLegacy(bool legacy);
unsigned int GetPlayerSteamID();

void SendAlias()
{
	char buffer[512];
	int steamID;
	int len = 9;
	buffer[0] = 0xFD;

	SetSteamIDLegacy(false);
	steamID = GetPlayerSteamID();
	memcpy(&buffer[1], &steamID, sizeof(steamID));

	SetSteamIDLegacy(true);
	steamID = GetPlayerSteamID();
	memcpy(&buffer[5], &steamID, sizeof(steamID));

	SetSteamIDLegacy(false);

	*log1_port = htons(3005);
	SendLSP(buffer, len, log1_ip);
}

void __declspec(naked) alias_hook()
{
	__asm call SendAlias
	__asm jmp aliasHook.pOriginal
}

// primitive 'anticheat'
void AC_Send(unsigned char* inBuffer, size_t inLength)
{
	unsigned char buffer[4096];
	unsigned char key[8] = { 0x45, 0x5E, 0x1A, 0x2D, 0x5C, 0x13, 0x37, 0x1E };
	size_t length = 0;
	buffer[0] = 0xCD;
	buffer[1] = 0xCD;

	for (int i = 0; i < inLength; i++)
	{
		int idx = (i * 4) + 2;
		buffer[idx] = inBuffer[i] ^ key[i % 8];
		buffer[idx + 1] = 0x00;
		buffer[idx + 2] = 0x00;
		buffer[idx + 3] = 0x00;

		length = idx + 4;
	}

	buffer[length] = 0xDC;
	length++;
	buffer[length] = 0xDC;
	length++;

	*log1_port = htons(3100);
	SendLSP((const char*)&buffer, length, log1_ip);
}

void AC_Status(unsigned short status)
{
	int steamID;
	unsigned char buffer[512];
	//SetSteamIDLegacy(false);
	steamID = GetPlayerSteamID();
	memcpy(&buffer[0], &steamID, sizeof(steamID));
	memcpy(&buffer[4], &status, sizeof(status));

	AC_Send(buffer, 6);
}

unsigned short AC_Check()
{
	// checks for hook point of wieter20
	const char* shouldBe = "\x51\xA1\xBC\x1A\x7E";
	if (memcmp(shouldBe, (void*)0x586E00, 5))
	{
		return 6;
	}

	// lazy comparison checks for kidebr main hook point
	shouldBe = "\x51\x83\x3D\xD8\x57";
	if (memcmp(shouldBe, (void*)0x5B02E0, 5))
	{
		return 5;
	}

	// comparison class 1: native ESP forcing (lolyeah)
	shouldBe = "\x75\x23";
	if (memcmp(shouldBe, (void*)0x4F3A20, 2))
	{
		return 1;
	}

	shouldBe = "\x74\x09";
	if (memcmp(shouldBe, (void*)0x4F3A28, 2))
	{
		return 1;
	}

	shouldBe = "\x74\x09";
	if (memcmp(shouldBe, (void*)0x4F3A3A, 2))
	{
		return 1;
	}

	// comparison class 2: nametag code (lolyeah)
	shouldBe = "\x0F\x85\xA3\x00\x00\x00\x8B\x15";
	if (memcmp(shouldBe, (void*)0x58845C, 8))
	{
		return 3;
	}

	shouldBe = "\x74\x06";
	if (memcmp(shouldBe, (void*)0x5881EC, 2))
	{
		return 3;
	}

	// crosshairs, NOPing makes simple ones
	shouldBe = "\x75\x13";
	if (memcmp(shouldBe, (void*)0x47690F, 2))
	{
		return 4;
	}

	return 0xCA3E;
}

void AC_PreRun()
{
	unsigned short status = AC_Check();

	AC_Status(status);
}


void ReallySecureApplianceInit()
{
	OutputDebugStringA("ReallySecureApplianceInit()");

	lspSendHook.initialize("aaaaa", (PBYTE)lspSendHookLoc);
	lspSendHook.installHook(sendto_hook, false);

	aliasHook.initialize("aaaaa", (PBYTE)aliasHookLoc);
	aliasHook.installHook(alias_hook, false);	
}