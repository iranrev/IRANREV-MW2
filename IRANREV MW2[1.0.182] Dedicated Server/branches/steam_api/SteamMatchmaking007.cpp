// ==========================================================
// alterIWnet project
// 
// Component: aiw_client
// Sub-component: steam_api
// Purpose: ISteamMatchmaking007 implementation
//
// Initial author: NTAuthority
// Started: 2010-09-27
// ==========================================================

#include "StdInc.h"
#include "SteamMatchmaking007.h"
#include "Callbacks.h"

// ^{[a-z0-9_]*} {.*}\({.*}\) \{

int CSteamMatchmaking007::GetFavoriteGameCount() {
	return 0;
}

bool CSteamMatchmaking007::GetFavoriteGame( int iGame, AppId_t *pnAppID, uint32 *pnIP, uint16 *pnConnPort, uint16 *pnQueryPort, uint32 *punFlags, RTime32 *pRTime32LastPlayedOnServer ) {
	return false;
}

int CSteamMatchmaking007::AddFavoriteGame( AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, RTime32 rTime32LastPlayedOnServer ) {
	return 0;
}

bool CSteamMatchmaking007::RemoveFavoriteGame( AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags ) {
	return false;
}

SteamAPICall_t CSteamMatchmaking007::RequestLobbyList() {
	SteamAPICall_t result = Callbacks::RegisterCall();
	LobbyMatchList_t* retvals = (LobbyMatchList_t*)malloc(sizeof(LobbyMatchList_t));
	retvals->m_nLobbiesMatching = 0;

	Callbacks::Return(retvals, LobbyMatchList_t::k_iCallback, result, sizeof(LobbyMatchList_t));

	return result;
}

void CSteamMatchmaking007::AddRequestLobbyListStringFilter( const char *pchKeyToMatch, const char *pchValueToMatch, ELobbyComparison eComparisonType ) {
	
}

void CSteamMatchmaking007::AddRequestLobbyListNumericalFilter( const char *pchKeyToMatch, int nValueToMatch, ELobbyComparison nComparisonType ) {
	
}

void CSteamMatchmaking007::AddRequestLobbyListNearValueFilter( const char *pchKeyToMatch, int nValueToBeCloseTo ) {
	
}

void CSteamMatchmaking007::AddRequestLobbyListFilterSlotsAvailable( int ) {
	
}

CSteamID CSteamMatchmaking007::GetLobbyByIndex( int iLobby ) {
	return CSteamID( 1337132, 0x40000, k_EUniversePublic, k_EAccountTypeChat );
}

SteamAPICall_t CSteamMatchmaking007::CreateLobby( ELobbyType eLobbyType, int ) {
	SteamAPICall_t result = Callbacks::RegisterCall();
	LobbyCreated_t* retvals = (LobbyCreated_t*)malloc(sizeof(LobbyCreated_t));
	CSteamID id = CSteamID( 1337132, 0x40000, k_EUniversePublic, k_EAccountTypeChat );
	//CSteamID id = CSteamID( (uint64)0x1100001034bd36e );

	retvals->m_eResult = k_EResultOK;
	retvals->m_ulSteamIDLobby = id.ConvertToUint64();

	//

	Callbacks::Return(retvals, LobbyCreated_t::k_iCallback, result, sizeof(LobbyCreated_t));

	JoinLobby(id);

	return result;
}

SteamAPICall_t CSteamMatchmaking007::JoinLobby( CSteamID steamIDLobby ) {
	SteamAPICall_t result = Callbacks::RegisterCall();
	LobbyEnter_t* retvals = (LobbyEnter_t*)malloc(sizeof(LobbyEnter_t));
	retvals->m_bLocked = false;
	retvals->m_EChatRoomEnterResponse = k_EChatRoomEnterResponseSuccess;
	retvals->m_rgfChatPermissions = (EChatPermission)0xFFFFFFFF;
	retvals->m_ulSteamIDLobby = steamIDLobby.ConvertToUint64();

	Callbacks::Return(retvals, LobbyEnter_t::k_iCallback, result, sizeof(LobbyEnter_t));

	return result;
}

void CSteamMatchmaking007::LeaveLobby( CSteamID steamIDLobby ) {
	
}

bool CSteamMatchmaking007::InviteUserToLobby( CSteamID steamIDLobby, CSteamID steamIDInvitee ) {
	
	return true;
}

int CSteamMatchmaking007::GetNumLobbyMembers( CSteamID steamIDLobby ) {
	//
	return 1;
}

CSteamID CSteamMatchmaking007::GetLobbyMemberByIndex( CSteamID steamIDLobby, int iMember ) {
	return CSteamID( 0xDED1CA7E, 1, k_EUniversePublic, k_EAccountTypeIndividual );
}

const char *CSteamMatchmaking007::GetLobbyData( CSteamID steamIDLobby, const char *pchKey ) {
	return "";
}

bool CSteamMatchmaking007::SetLobbyData( CSteamID steamIDLobby, const char *pchKey, const char *pchValue ) {
	return true;
}

int CSteamMatchmaking007::GetLobbyDataCount( CSteamID steamIDLobby ) {
	
	return 0;
}

bool CSteamMatchmaking007::GetLobbyDataByIndex( CSteamID steamIDLobby, int iData, char *pchKey, int cubKey, char *pchValue, int cubValue ) {
	
	return false;
}

bool CSteamMatchmaking007::DeleteLobbyData( CSteamID steamIDLobby, const char *pchKey ) {
	
	return 0;
}

const char * CSteamMatchmaking007::GetLobbyMemberData( CSteamID steamIDLobby, CSteamID steamIDUser, const char *pchKey ) {
	
	return "";
}

void CSteamMatchmaking007::SetLobbyMemberData( CSteamID steamIDLobby, const char *pchKey, const char *pchValue ) {
	
}

bool CSteamMatchmaking007::SendLobbyChatMsg( CSteamID steamIDLobby, const void *pvMsgBody, int cubMsgBody ) {
	
	return true;
}

int CSteamMatchmaking007::GetLobbyChatEntry( CSteamID steamIDLobby, int iChatID, CSteamID *pSteamIDUser, void *pvData, int cubData, EChatEntryType *peChatEntryType ) {
	
	return 0;
}

bool CSteamMatchmaking007::RequestLobbyData( CSteamID steamIDLobby ) {
	
	return false;
}

void CSteamMatchmaking007::SetLobbyGameServer( CSteamID steamIDLobby, uint32 unGameServerIP, uint16 unGameServerPort, CSteamID steamIDGameServer ) {
	
}

bool CSteamMatchmaking007::GetLobbyGameServer( CSteamID steamIDLobby, uint32 *punGameServerIP, uint16 *punGameServerPort, CSteamID *psteamIDGameServer ) {
	
	return false;
}

bool CSteamMatchmaking007::SetLobbyMemberLimit( CSteamID steamIDLobby, int cMaxMembers ) {
	
	return true;
}

int CSteamMatchmaking007::GetLobbyMemberLimit( CSteamID steamIDLobby ) {
	
	return 0;
}

bool CSteamMatchmaking007::SetLobbyType( CSteamID steamIDLobby, ELobbyType eLobbyType ) {
	
	return true;
}

bool CSteamMatchmaking007::SetLobbyJoinable( CSteamID steamIDLobby, bool bJoinable ) {
	
	return true;
}

CSteamID CSteamMatchmaking007::GetLobbyOwner( CSteamID steamIDLobby ) {
	return CSteamID( 0xDED1CA7E, 1, k_EUniversePublic, k_EAccountTypeIndividual );
}

bool CSteamMatchmaking007::SetLobbyOwner( CSteamID steamIDLobby, CSteamID steamIDNewOwner ) {
	return true;
}
