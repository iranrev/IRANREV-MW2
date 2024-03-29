#include "stdafx.h"
#include "Hooking.h"

#pragma unmanaged
void CallHook::initialize(const char* pOriginalCode, PBYTE place)
{
	pPlace = place;
	memcpy(bOriginalCode, pPlace, sizeof(bOriginalCode));
	pOriginal = place + sizeof(bOriginalCode) + *(ptrdiff_t*) (bOriginalCode + 1);
}

int CallHook::installHook(void (*hookToInstall)(), bool unprotect)
{
	DWORD d = 0;

	//if (memcmp(bOriginalCode, pPlace, sizeof(bOriginalCode)))
		//return 0;
	//if (unprotect &&
	//	!VirtualProtect(pPlace + 1, sizeof(ptrdiff_t), PAGE_READWRITE, &d))
	//	return 0;
	*(ptrdiff_t*) (pPlace + 1) = (PBYTE) hookToInstall - pPlace - sizeof(bOriginalCode);
	//if (unprotect)
	//	VirtualProtect(pPlace + 1, sizeof(ptrdiff_t), d, &d);
	return 1;
}

int CallHook::releaseHook(bool unprotect)
{
	DWORD d = 0;

	if (unprotect &&
		!VirtualProtect(pPlace + 1, sizeof(ptrdiff_t), PAGE_READWRITE, &d))
		return 0;
	memcpy(pPlace + 1, bOriginalCode + 1, sizeof(bOriginalCode) - 1);
	if (unprotect)
		VirtualProtect(pPlace + 1, sizeof(ptrdiff_t), d, &d);
	return 1;
}

///////////////////////////////////////////////////////////////////////
void CallHook4D1::initialize(DWORD place, void *hookToInstall)
{
	pPlace = (PBYTE)place;
	memcpy(bOriginalCode, pPlace, sizeof(bOriginalCode));
	pOriginal = pPlace + sizeof(bOriginalCode) + *(ptrdiff_t*) (bOriginalCode + 1);
	hook = hookToInstall;
}

void CallHook4D1::installHook(void *hookToInstall)
{
	if (hookToInstall) hook = hookToInstall;
	if (hook) {
		*(ptrdiff_t*) (pPlace + 1) = (PBYTE) hook - pPlace - 5;
	}
}

void CallHook4D1::releaseHook()
{
	//memcpy(pPlace + 1, bOriginalCode + 1, sizeof(bOriginalCode) - 1);
	*(ptrdiff_t*) (pPlace + 1) = (PBYTE) pOriginal - pPlace - 5;
}
///////////////////////////////////////////////////////////////////////
void PointerHook::initialize(PVOID* place)
{
	pPlace = place;
	pOriginal = NULL;
}

int PointerHook::installHook(void (*hookToInstall)(), bool unprotect)
{
	DWORD d = 0;

	if (pOriginal)
		return 0;
	if (unprotect &&
		!VirtualProtect(pPlace, sizeof(PVOID), PAGE_READWRITE, &d))
		return 0;
	pOriginal = *pPlace;
	*pPlace = (PVOID) hookToInstall;
	if (unprotect)
		VirtualProtect(pPlace, sizeof(PVOID), d, &d);
	return 1;
}

int PointerHook::releaseHook(bool unprotect)
{
	DWORD d = 0;

	if (!pOriginal)
		return 0;
	if (unprotect &&
		!VirtualProtect(pPlace, sizeof(PVOID), PAGE_READWRITE, &d))
		return 0;
	*pPlace = pOriginal;
	pOriginal = NULL;
	if (unprotect)
		VirtualProtect(pPlace, sizeof(PVOID), d, &d);
	return 1;
}

void StompHook::initialize(const char* pOriginalCode, BYTE countBytes, PBYTE place)
{
	pPlace = place;
	bCountBytes = countBytes < sizeof(bOriginalCode) ? countBytes : sizeof(bOriginalCode);
	memcpy(bOriginalCode, pPlace, bCountBytes);
}


int StompHook::installHook(void (*hookToInstall)(), bool useJump, bool unprotect)
{
	DWORD d = 0;

	//if (bCountBytes < 5 || memcmp(bOriginalCode, pPlace, bCountBytes))
	//	return 0;
	if (unprotect &&
		!VirtualProtect(pPlace, bCountBytes, PAGE_READWRITE, &d))
		return 0;
	memset(pPlace, NOP, bCountBytes);
	pPlace[0] = useJump ? JMP_NEAR32 : CALL_NEAR32;
	*(ptrdiff_t*) (pPlace + 1) = (PBYTE) hookToInstall - pPlace - 5;
	if (unprotect)
		VirtualProtect(pPlace, bCountBytes, d, &d);
	return 1;
}

int StompHook::releaseHook(bool unprotect)
{
	DWORD d = 0;

	if (unprotect &&
		!VirtualProtect(pPlace, bCountBytes, PAGE_READWRITE, &d))
		return 0;
	memcpy(pPlace, bOriginalCode, bCountBytes);
	if (unprotect)
		VirtualProtect(pPlace, bCountBytes, d, &d);
	return 1;
}

void _nop(void* pAddress, DWORD size)
{
	memset(pAddress, 0x90, size);
	return;

	DWORD dwAddress = (DWORD)pAddress;
	if ( size % 2 )
	{
		*(BYTE*)pAddress = 0x90;
		dwAddress++;
	}
	if ( size - ( size % 2 ) )
	{
		DWORD sizeCopy = size - ( size % 2 );
		do
		{
			*(WORD*)dwAddress = 0xFF8B;
			dwAddress += 2;
			sizeCopy -= 2;
		}
		while ( sizeCopy );	
	}
}