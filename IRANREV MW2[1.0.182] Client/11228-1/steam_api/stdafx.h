#pragma once
#define _CRT_SECURE_NO_WARNINGS

#define DEDICATED 0
//#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS

// Windows headers
#define WIN32_LEAN_AND_MEAN
#define ReadDirectoryChangesW ReadDirectoryChangesW__
#define CreateRemoteThread CreateRemoteThread__
#include <windows.h>
#undef CreateRemoteThread
#undef ReadDirectoryChangesW

#if D3D_EXPERIMENTS
#include <d3d9.h>
#else
struct IDirect3DVertexShader9;
struct IDirect3DPixelShader9;
struct IDirect3DTexture9;
struct IDirect3DVertexDeclaration9;
struct IDirect3DDevice9;
#endif

#include "steamworks.h"
//#include "steam_api.h"
#include <iostream>
#include <fstream>
//#include <boost/program_options.hpp>
//#include <boost/unordered_map.hpp>
#include <vector>

// MW2 structures
#include "mw2.h"

// Hooking
//#include "Hooking.h"

#pragma unmanaged
#include "libtomcrypt/tomcrypt.h"

#include <dwmapi.h>
#pragma managed