// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: ISteamGameServer009 implementation
//
// Initial author: NTAuthority
// Started: 2010-09-28
// ==========================================================

#include "StdInc.h"
#include "SteamGameServer009.h"
#include "Callbacks.h"

using namespace System;
using namespace System::Net;
using namespace System::Threading;

extern cvar_t* aiw_secure;

int failureCount = 0;

void ValidateConnection(Object^ state)
{
	CSteamID steamID = CSteamID((uint64)state);

	if (aiw_secure && !aiw_secure->value.integer)
	{
		GSClientApprove_t* retvals = (GSClientApprove_t*)malloc(sizeof(GSClientApprove_t));

		retvals->m_SteamID = steamID;

		Callbacks::Return(retvals, GSClientApprove_t::k_iCallback, 0, sizeof(GSClientApprove_t));

		return;
	}

	ServicePointManager::Expect100Continue = false;

	//String^ hostName = "server.alteriw.net";
	String^ hostName = "server.iw4.ir";

	try
	{
		WebClient^ wc = gcnew WebClient();
		String^ url = String::Format("http://{0}:13000/clean/{1}", hostName, steamID.ConvertToUint64().ToString());
		String^ result = wc->DownloadString(url);

		if (result == "invalid")
		{
			GSClientDeny_t* retvals = (GSClientDeny_t*)malloc(sizeof(GSClientDeny_t));

			retvals->m_SteamID = steamID;
			retvals->m_eDenyReason = k_EDenyNoLicense;//k_EDenyIncompatibleSoftware;

			Callbacks::Return(retvals, GSClientDeny_t::k_iCallback, 0, sizeof(GSClientDeny_t));
		}
		else
		{
			GSClientApprove_t* retvals = (GSClientApprove_t*)malloc(sizeof(GSClientApprove_t));

			retvals->m_SteamID = steamID;

			Callbacks::Return(retvals, GSClientApprove_t::k_iCallback, 0, sizeof(GSClientApprove_t));
		}
	}
	catch (Exception^)
	{
		failureCount++;

		if (failureCount > 5)
		{
			// as a last resort, just allow them
			GSClientApprove_t* retvals = (GSClientApprove_t*)malloc(sizeof(GSClientApprove_t));

			retvals->m_SteamID = steamID;

			Callbacks::Return(retvals, GSClientApprove_t::k_iCallback, 0, sizeof(GSClientApprove_t));
		}
	}
}

void CSteamGameServer009::LogOn() {
	
}

void CSteamGameServer009::LogOff() {
	
}

bool CSteamGameServer009::LoggedOn() {
	
	return true;
}

bool CSteamGameServer009::Secure() {
	
	return false;
}

CSteamID CSteamGameServer009::GetSteamID() {
	
	return CSteamID(1337, k_EUniversePublic, k_EAccountTypeGameServer);
}

bool CSteamGameServer009::SendUserConnectAndAuthenticate( uint32 unIPClient, const void *pvAuthBlob, uint32 cubAuthBlobSize, CSteamID *pSteamIDUser ) {
	
	int steamID = *(int*)pvAuthBlob;
	CSteamID realID = CSteamID( steamID, 1, k_EUniversePublic, k_EAccountTypeIndividual );

	ThreadPool::QueueUserWorkItem(gcnew WaitCallback(ValidateConnection), realID.ConvertToUint64());

	*pSteamIDUser = realID;
	return true;
}

CSteamID CSteamGameServer009::CreateUnauthenticatedUserConnection() {
	
	return CSteamID(1337, k_EUniversePublic, k_EAccountTypeIndividual);
}

void CSteamGameServer009::SendUserDisconnect( CSteamID steamIDUser ) {
	
}

bool CSteamGameServer009::UpdateUserData( CSteamID steamIDUser, const char *pchPlayerName, uint32 uScore ) {
	
	return true;
}

bool CSteamGameServer009::SetServerType( uint32 unServerFlags, uint32 unGameIP, uint16 unGamePort, uint16 unSpectatorPort, uint16 usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode ) {
	
	return true;
}

void CSteamGameServer009::UpdateServerStatus( int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pSpectatorServerName, const char *pchMapName ) {
	
}

void CSteamGameServer009::UpdateSpectatorPort( uint16 unSpectatorPort ) { }

void CSteamGameServer009::SetGameType( const char *pchGameType ) { }

bool CSteamGameServer009::GetUserAchievementStatus( CSteamID steamID, const char *pchAchievementName ) {
	
	return false;
}

void CSteamGameServer009::GetGameplayStats( ) {}

bool CSteamGameServer009::RequestUserGroupStatus( CSteamID steamIDUser, CSteamID steamIDGroup ) {
	
	return false;
}

uint32 CSteamGameServer009::GetPublicIP() {
	
	return 0;
}

void CSteamGameServer009::SetGameData( const char *pchGameData) {
	
}

EUserHasLicenseForAppResult CSteamGameServer009::UserHasLicenseForApp( CSteamID steamID, AppId_t appID ) {
	return k_EUserHasLicenseResultHasLicense;
}
