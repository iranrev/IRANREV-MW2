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
// Rewritten for Alteriwnet(MW2 1.0.182) by Hosseeinpourziyaie
// Started: 2012-10-20 - Ported on 2017 by Hosseeinpourziyaie
// =============================================================

#include "stdafx.h"
#include "Hooking.h"
#pragma unmanaged

#define Version "1.8.0"
char buildID[1024];

typedef int (__cdecl * R_TextWidth_t)(const char* text, int maxlength, Font* font);
R_TextWidth_t R_TextWidth = (R_TextWidth_t)0x50BEC0; 

typedef void* (__cdecl * R_RegisterFont_t)(const char* asset);
R_RegisterFont_t R_RegisterFont = (R_RegisterFont_t)0x50BE70;

typedef void (__cdecl * R_AddCmdDrawText_t)(const char* text, int, void* font, float screenX, float screenY, float, float, float rotation, float* color, int);
R_AddCmdDrawText_t R_AddCmdDrawText = (R_AddCmdDrawText_t)0x510710 ; // i found it once before but it was wrong : 0x421660

CallHook4D1 drawDevStuffHook;
DWORD drawDevStuffHookLoc = 0x5B0529;


int R_GetScaledWidth(const char* text, float sizeX, void* font)
{
	if (!R_TextWidth)
	{
		return 0;
	}

	int normalWidth = R_TextWidth(text, 0x7FFFFFFF, (Font*)font);
	double scaledWidth = normalWidth * sizeX;

	return (int)scaledWidth;
}

typedef void (__cdecl * Com_Printf_t)(int, const char*, ...);
extern Com_Printf_t Com_Printf;

void DrawBranding()
{
	//if (!cg_hideVersion->current.boolean)
	{
		float font_size = 0.85;//0.7f
		void* font = R_RegisterFont("fonts/normalFont");

		int textWidth = R_GetScaledWidth("^5IRAN REVOLUTION", font_size, font);

		auto width = [] (int offset, int width)
		{
			return *(int*)0x673E188 - ( offset + width ); //0x673E188
		};

		float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		if (CL_IsCgameInitialized())
		{
			color[3] = 0.3f;
		}

		R_AddCmdDrawText("^5IRAN REVOLUTION", 0x7FFFFFFF, font, width(10, textWidth), 30, font_size, font_size, 0.0f, color, 0);
	
	}

}
void __declspec(naked) DrawDevStuffHookStub()
{
	__asm
	{
		call DrawBranding
		jmp drawDevStuffHook.pOriginal
	}
}

void PatchMW2_Branding()
{
	// Not working and I Want to Sleep (zzz)
	drawDevStuffHook.initialize(drawDevStuffHookLoc, DrawDevStuffHookStub);
	drawDevStuffHook.installHook();

	//Commented due to : We dont want to create Extra folders
	// video file path
	//*(DWORD*)0x522A04 = (DWORD)"%s\\GameData\\Videos\\%s.bik";

	// always play the intro video
	nop(0x60E55F, 2);

	*(DWORD*)0x60E562 = (DWORD)"unskippablecinematic IW_logo\n";

	strcpy((char*)0x7859C0, "Iran Revolution"); // was "alterIWnet"
	strcpy((char*)0x6EC9C4, "Iran Revolution Console"); // was "aIW Console"

	// patch build string
	sprintf(buildID, "^5IRAN REVOLUTION MW2 %s\n^2www.iw4.ir" , Version);
	*(DWORD*)0x5037EB = (DWORD)&buildID;

	// increase font sizes for chat on higher resolutions
	*(BYTE*)0x58768A = 0xEB;
	*(BYTE*)0x5876A4 = 0xEB;
	
	/*static float float13 = 10.0f;
	static float float10 = 7.0f;

	*(float**)0x58768E = &float13;
	*(float**)0x5876A8 = &float10;*/
}