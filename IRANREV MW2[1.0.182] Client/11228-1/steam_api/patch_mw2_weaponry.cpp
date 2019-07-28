#include "stdafx.h"
#include "Hooking.h"

#include <direct.h>

#pragma unmanaged
#if crap

typedef struct weaponEntry_s
{
	const char* name;
	int offset;
	int type;
} weaponEntry_t;

#define NUM_ENTRIES 672

weaponEntry_t* weaponEntries = (weaponEntry_t*)0x795F00;

void DumpEntry(FILE* file, int type, BYTE* data)
{
	switch (type)
	{
		case 0: // string
			{
				char* str = (char*)(*(DWORD_PTR*)data);

				if (str && (int)str != 0xFFFFFFFF && (int)str > 16)
				{
					fprintf(file, "%s", str);
				}
				break;
			}
		case 7:
			{
				float number = *(float*)data;

				fprintf(file, "%g", number);
				break;
			}
		case 11: // xmodel
			{
				BYTE* model = (BYTE*)(*(DWORD_PTR*)data);

				if (model)
				{
					char* str = (char*)(*(DWORD_PTR*)model);

					if (str)
					{
						fprintf(file, "%s", str);
					}
				}
				break;
			}
	}
}

void DumpWeaponFile(FILE* file, BYTE* data)
{
	for (int i = 0; i < NUM_ENTRIES; i++)
	{
		weaponEntry_t* entry = &weaponEntries[i];

		fprintf(file, "%s\\", entry->name);

		DumpEntry(file, entry->type, (data + entry->offset));

		fprintf(file, "\\");
	}
}

void DumpWeaponTypes(FILE* file)
{
	for (int i = 0; i < NUM_ENTRIES; i++)
	{
		weaponEntry_t* entry = &weaponEntries[i];

		//fprintf(file, "%s\\", entry->name);

		//DumpEntry(file, entry->type, (data + entry->offset));

		fprintf(file, "+0x%x:\t%s", entry->offset, entry->name);

		switch (entry->type)
		{
			case 0:
				fprintf(file, " (string)");
				break;
			case 7:
				fprintf(file, " (float)");
				break;
			case 8:
				fprintf(file, " (float / 17.6)");
				break;
			case 9:
				fprintf(file, " (float / 1000.0)");
				break;
			case 11:
				fprintf(file, " (xmodel)");
				break;
			case 0xD:
				fprintf(file, " (phys collmap)");
				break;
			default:
				fprintf(file, " (type %d)", entry->type);
				break;
		}

		fprintf(file, "\n");
	}
}

StompHook weaponFileHook;
DWORD weaponFileHookLoc = 0x581900;

typedef void* (__cdecl * LoadAssetOfType_t)(int type, const char* filename);
LoadAssetOfType_t LoadAssetOfType = (LoadAssetOfType_t)0x493FB0;

void* WeaponFileHookFunc(const char* filename)
{
	BYTE* file = (BYTE*)LoadAssetOfType(0x1C, filename);

	_mkdir("raw\\weapons\\mp");

	char dumpfile[512];
	strcpy(dumpfile, "raw\\weapons\\mp\\");
	strcat(dumpfile, filename);

	FILE* dump = fopen(dumpfile, "w");
	fprintf(dump, "WEAPONFILE\\");
	
	DumpWeaponFile(dump, file);

	fclose(dump);

	dump = fopen("raw\\weaponFields.txt", "w");
	DumpWeaponTypes(dump);
	fclose(dump);

	//TerminateProcess(GetCurrentProcess(), 0);

	return file;
}

void __declspec(naked) WeaponFileHookStub()
{
	__asm jmp WeaponFileHookFunc
}

void WeaponryInit()
{
	weaponFileHook.initialize("aaaaa", 5, (PBYTE)weaponFileHookLoc);
	weaponFileHook.installHook(WeaponFileHookStub, true, false);
}
#endif