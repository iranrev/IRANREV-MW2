// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: Basic Steam interface functions for usage in main
//
// Initial author: NTAuthority
// Started: 2010-09-10
// ==========================================================

#pragma once

// all interface IDs supported
typedef enum SteamInterface_t {
	INTERFACE_STEAMCLIENT008,
	INTERFACE_STEAMUSER012,
	INTERFACE_STEAMREMOTESTORAGE002,
	INTERFACE_STEAMUTILS005,
	INTERFACE_STEAMNETWORKING003,
	INTERFACE_STEAMFRIENDS005,
	INTERFACE_STEAMMATCHMAKING007,
	INTERFACE_STEAMGAMESERVER009,
	INTERFACE_STEAMMASTERSERVERUPDATER001
};

// basic class
public class CSteamBase {
private:
	static std::map<SteamInterface_t, void*> _instances;
private:
	static void* CreateInterface(SteamInterface_t interfaceID);
public:
	// get interface instance from identifier
	static void* GetInterface(SteamInterface_t interfaceID);

	// run callbacks
};