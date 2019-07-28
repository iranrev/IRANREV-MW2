#pragma once

/*typedef void (*DB_LoadXAssets_t)(XZoneInfo* data, int count, int unknown);
extern DB_LoadXAssets_t DB_LoadXAssets;*/

typedef void* (__cdecl * DB_FindXAssetHeader_t)(int type, const char* filename);
extern DB_FindXAssetHeader_t DB_FindXAssetHeader;

typedef struct  
{
	char type;
	char pad[3];
	const char* folder;
	const char* file;
} StreamFile;

typedef struct  
{
	char pad[20];
	StreamFile* stream;
	char pad2[76];
} snd_alias_t;

typedef struct  
{
	const char* name;
	snd_alias_t* aliases;
	int numAliases;
} snd_alias_list_t;

typedef enum assetType_e
{
	ASSET_TYPE_PHYSPRESET = 0,
	ASSET_TYPE_PHYS_COLLMAP = 1,
	ASSET_TYPE_XANIM = 2,
	ASSET_TYPE_XMODELSURFS = 3,
	ASSET_TYPE_XMODEL = 4,
	ASSET_TYPE_MATERIAL = 5,
	ASSET_TYPE_PIXELSHADER = 6,
	ASSET_TYPE_VERTEXSHADER = 7,
	ASSET_TYPE_VERTEXDECL = 8,
	ASSET_TYPE_TECHSET = 9,
	ASSET_TYPE_IMAGE = 10,
	ASSET_TYPE_SOUND = 11,
	ASSET_TYPE_SNDCURVE = 12,
	ASSET_TYPE_LOADED_SOUND = 13,
	ASSET_TYPE_COL_MAP_SP = 14,
	ASSET_TYPE_COL_MAP_MP = 15,
	ASSET_TYPE_COM_MAP = 16,
	ASSET_TYPE_GAME_MAP_SP = 17,
	ASSET_TYPE_GAME_MAP_MP = 18,
	ASSET_TYPE_MAP_ENTS = 19,
	ASSET_TYPE_FX_MAP = 20,
	ASSET_TYPE_GFX_MAP = 21,
	ASSET_TYPE_LIGHTDEF = 22,
	ASSET_TYPE_UI_MAP = 23,
	ASSET_TYPE_FONT = 24,
	ASSET_TYPE_MENUFILE = 25,
	ASSET_TYPE_MENU = 26,
	ASSET_TYPE_LOCALIZE = 27,
	ASSET_TYPE_WEAPON = 28,
	ASSET_TYPE_SNDDRIVERGLOBALS = 29,
	ASSET_TYPE_FX = 30,
	ASSET_TYPE_IMPACTFX = 31,
	ASSET_TYPE_AITYPE = 32,
	ASSET_TYPE_MPTYPE = 33,
	ASSET_TYPE_CHARACTER = 34,
	ASSET_TYPE_XMODELALIAS = 35,
	ASSET_TYPE_RAWFILE = 36,
	ASSET_TYPE_STRINGTABLE = 37,
	ASSET_TYPE_LEADERBOARDDEF = 38,
	ASSET_TYPE_STRUCTUREDDATADEF = 39,
	ASSET_TYPE_TRACER = 40,
	ASSET_TYPE_VEHICLE = 41,
	ASSET_TYPE_ADDON_MAP_ENTS = 42,
	ASSET_TYPE_MAX = 43
} assetType_t;

// material stuff
struct GfxImage
{
        char* texture;
        char unknown2;
        char a3;
        char a2;
        char unknown3;
        char unknown4;
        char unknown5;
        char unknown6;
        char a4;
        int dataLength1;
        int dataLength2;
        short height;
		short width;
        short depth;
        short unknown8;
        char* name;
};

struct MaterialTextureDef
{
	unsigned int typeHash; // asset hash of type
	char firstCharacter; // first character of image name
	char secondLastCharacter; // second-last character of image name (maybe only in CoD4?!)
	unsigned char unknown; // maybe 0xE2
	char unknown2; // likely 0x00
	GfxImage* image; // GfxImage* actually
};

struct VertexDecl
{
	const char* name;
	int unknown;
	char pad[28];
	IDirect3DVertexDeclaration9* declarations[16];
};

struct PixelShader
{
	const char* name;
	IDirect3DPixelShader9* shader;
	DWORD* bytecode;
	int flags;
};

struct VertexShader
{
	const char* name;
	IDirect3DVertexShader9* shader;
	DWORD* bytecode;
	int flags;
};

struct MaterialPass
{
	VertexDecl* vertexDecl;
	VertexShader* vertexShader;
	PixelShader* pixelShader;
	char pad[8];
};

struct MaterialTechnique
{
	int pad;
	short pad2;
	short numPasses;
	MaterialPass passes[1];
};

struct MaterialTechniqueSet
{
	const char* name;
	char pad[4];
	MaterialTechniqueSet* remappedTechniqueSet;
	MaterialTechnique* techniques[48];
};

struct MaterialStuff
{
	char unk;
	char sortKey;
};

struct Material
{
	const char* name;
	union
	{
		unsigned short flags; // 0x2F00 for instance
		MaterialStuff sort;
	};
	unsigned char animationX; // amount of animation frames in X
	unsigned char animationY; // amount of animation frames in Y
	char unknown1[4]; // 0x00
	unsigned int rendererIndex; // only for 3D models
	char unknown9[4];
	unsigned int surfaceTypeBits;
	unsigned int unknown2; // 0xFFFFFFFF
	unsigned int unknown3; // 0xFFFFFF00
	char unknown4[40]; // 0xFF
	char numMaps; // 0x01, possibly 'map count' (zone code confirms)
	char unknown5; // 0x00
	char unknownCount2; // 0x01, maybe map count actually
	char unknown6; // 0x03
	unsigned int unknown7; // 0x04
	MaterialTechniqueSet* techniqueSet; // '2d' techset; +80
	MaterialTextureDef* maps; // map references
	unsigned int unknown8;
	void* stateMap; // might be NULL, need to test
};

typedef struct fontEntry_s
{
	unsigned short character;
	unsigned char padLeft;
	unsigned char padTop;
	unsigned char padRight;
	unsigned char width;
	unsigned char height;
	unsigned char const0;
	float uvLeft;
	float uvTop;
	float uvRight;
	float uvBottom;
} fontEntry_t;

typedef struct Font_s
{
	char* name;
	int size;
	int entries;
	Material* image;
	Material* glowImage;
	fontEntry_t* characters;
} Font;


typedef int (__cdecl * CL_IsCgameInitialized_t)();
extern CL_IsCgameInitialized_t CL_IsCgameInitialized;

