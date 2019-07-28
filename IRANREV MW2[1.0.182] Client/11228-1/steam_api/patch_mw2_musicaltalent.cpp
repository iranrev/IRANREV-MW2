// =============================================================
// Iran Revolution Mw2 Project
//   Version 182 Steam = Based on Alteriwnet Source code
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: The will of a single man. Makarov's men, fighting
//          Shepard's men. That, or a beautiful bridge...
//          about to be destroyed by some weird warfare.
//
// Initial author: NTAuthority
// Converted for Alteriwnet(182) by Hosseeinpourziyaie
// Started: 2012-10-20 - Ported on 2017 by Hosseeinpourziyaie
// =============================================================

#include "stdafx.h"
#include "Hooking.h"
#pragma unmanaged

//CallHook findSoundAliasHook;
CallHook4D1 findSoundAliasHook;
DWORD findSoundAliasHookLoc = 0x644AE9;

snd_alias_list_t* FindSoundAliasHookFunc(assetType_t type, const char* name)
{
	snd_alias_list_t* aliases = (snd_alias_list_t*)DB_FindXAssetHeader(type, name);
	
	// the most hated sound at this point
	if (!_stricmp(name, "music_mainmenu_mp"))
	{
		static snd_alias_list_t newList;
		static snd_alias_t newAlias;
		static StreamFile newFile;

		// duplicate the asset as we can't modify pointers
		memcpy(&newList, aliases, sizeof(newList));

		memcpy(&newAlias, newList.aliases, sizeof(newAlias));
		newList.aliases = &newAlias;

		memcpy(&newFile, newAlias.stream, sizeof(newFile));
		newAlias.stream = &newFile;
		// and replace the filename.
		newFile.file = "hz_boneyard_intro_LR_1.mp3";

		// oh and this too
		aliases = &newList;
	}

	return aliases;
}
/*
void __declspec(naked) FindSoundAliasHookStub()
{
	__asm
	{
		call FindSoundAliasHookFunc
	}
}*/


void MusicalTalentInit()
{
	findSoundAliasHook.initialize(findSoundAliasHookLoc, FindSoundAliasHookFunc);
	findSoundAliasHook.installHook();

}