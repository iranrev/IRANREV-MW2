// ==========================================================
// Iran Revolution Mw2 Project
//   Version 182 Steam = Based on Alteriwnet Source code
// 
// Component: clientdll
// Sub-component: steam_api
// Purpose: DLL library entry point, SteamAPI exports
//
// Initial author: NTAuthority
// Started: Actually You Must Himselve
// ==========================================================

// steam_api_emu.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#include "SteamclientAPI.h"
#include "SteamAPI.h"
#include "AppTools.h"
#include "IniReader.h"
#include "SteamAPIHook.h"
#include <detours.h>
#include <setupapi.h>
#include <devguid.h>
#include "callbacks.h"
#include "diskinfo.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Xml::Linq;
using namespace System::Net;
using namespace System::Net::NetworkInformation;
using namespace System::Reflection;
using namespace System::Threading;

unsigned int hash(unsigned char* inpStr, size_t len)
{
	unsigned int value = 0,temp = 0;
	for(size_t i=0;i<len;i++)
	{
		temp = inpStr[i];
		temp += value;
		value = temp << 10;
		temp += value;
		value = temp >> 6;
		value ^= temp;
	}
	temp = value << 3;
	temp += value;
	unsigned int temp2 = temp >> 11;
	temp = temp2 ^ temp;
	temp2 = temp << 15;
	value = temp2 + temp;
	if(value < 2) value += 2;
	return value;
}

Microsoft::Win32::RegistryKey^ GetNetworkRegistryKey(String^ id) {
	try
	{
		Microsoft::Win32::RegistryKey^ networkInterfaceKey = Microsoft::Win32::Registry::LocalMachine->OpenSubKey("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}", false);
		cli::array<String^>^ keyNames = networkInterfaceKey->GetSubKeyNames();

		for each (String^ keyName in keyNames) {
			Microsoft::Win32::RegistryKey^ key = networkInterfaceKey->OpenSubKey(keyName);

			String^ value = (String^)key->GetValue("NetCfgInstanceId", "");
			if (value == id) {
				return key;
			}
		}

		return nullptr;
	}
	catch (System::Security::SecurityException^)
	{
		return nullptr;
	}
}

String^ GetDeviceIDForDriverKey(String^ key)
{
	int index = 0;
	char buffer[1024];
	String^ deviceID = "";

	SP_DEVINFO_DATA data;
	memset(&data, 0, sizeof(data));
	data.cbSize = sizeof(data);

	HDEVINFO handle = SetupDiGetClassDevs(&GUID_DEVCLASS_NET, NULL, NULL, DIGCF_PRESENT);

	while (SetupDiEnumDeviceInfo(handle, index, &data))
	{
		index++;

		if (SetupDiGetDeviceRegistryPropertyA(handle, &data, SPDRP_DRIVER, NULL, (PBYTE)buffer, sizeof(buffer), NULL))
		{
			String^ key2 = gcnew String(buffer);

			if (key->Replace("HKEY_LOCAL_MACHINE\\", "")->ToLower() == key2->ToLower())
			{
				if (SetupDiGetDeviceRegistryPropertyA(handle, &data, SPDRP_HARDWAREID, NULL, (PBYTE)buffer, sizeof(buffer), NULL))
				{
					deviceID = gcnew String(buffer);
				}
			}
		}
	}

	int err = GetLastError();

	if (handle)
	{
		SetupDiDestroyDeviceInfoList(handle);
	}

	return deviceID;
}
/*
bool IsValidInterface(String^ id) {
if (Environment::OSVersion->Platform != PlatformID::Win32Windows && Environment::OSVersion->Platform != PlatformID::Win32NT) {
return true;
}
*/

bool IsValidInterface(String^ id, bool legacy) {
	if (Environment::OSVersion->Platform != PlatformID::Win32Windows && Environment::OSVersion->Platform != PlatformID::Win32NT) {
		return true;
	}

	Microsoft::Win32::RegistryKey^ key = GetNetworkRegistryKey(id);

	if (key == nullptr) {
		return false;
	}

	String^ deviceID = GetDeviceIDForDriverKey(key->Name->Replace("SYSTEM\\CurrentControlSet\\Control\\Class\\", ""));//(String^)key->GetValue("MatchingDeviceId", "");
	String^ deviceID2 = "";//(String^)key->GetValue("DeviceInstanceId", "");

	if (legacy)
	{
		deviceID = (String^)key->GetValue("DeviceInstanceId", "");

		return (deviceID->ToLower()->StartsWith("pci"));
	}

	key->Close();

	return (deviceID->ToLower()->StartsWith("pci") || deviceID->ToLower()->StartsWith("usb") || deviceID->ToLower()->StartsWith("{") || deviceID2->ToLower()->StartsWith("pci"));
}

String^ WhyInvalidInterface(String^ id) {
	if (Environment::OSVersion->Platform != PlatformID::Win32Windows && Environment::OSVersion->Platform != PlatformID::Win32NT) {
		return "BECAUSE I'M COOL";
	}

	Microsoft::Win32::RegistryKey^ key = GetNetworkRegistryKey(id);

	if (key == nullptr) {
		return "NULLPTR";
	}

	String^ deviceID = (String^)key->GetValue("MatchingDeviceId", "");
	String^ deviceID2 = (String^)key->GetValue("DeviceInstanceId", "");
	String^ deviceID3 = GetDeviceIDForDriverKey(key->Name->Replace("SYSTEM\\CurrentControlSet\\Control\\Class\\", ""));

	key->Close();

	return ("#1: " + deviceID->ToLower() + " #2: " + deviceID2->ToLower() + " #3 (AwesomeID): " + deviceID3->ToLower());
}

char* lolololol = "ATTN: developer of that weird registry hook\r\nI'm sorry, but it seems you didn't read rule #724865. Read it, apply it, and maybe I'll give up. By the way, your actions did have some results: banning doesn't result in a ban anymore.";

bool IsConnectedInterface(String^ id) {
	if (Environment::OSVersion->Platform != PlatformID::Win32Windows && Environment::OSVersion->Platform != PlatformID::Win32NT) {
		return true;
	}

	Microsoft::Win32::RegistryKey^ key = GetNetworkRegistryKey(id);

	if (key == nullptr) {
		return false;
	}

	cli::array<String^>^ values = key->GetValueNames();
	String^ valueName = "";
	bool hasProviderName = false;

	for each (String^ value in values) {
		if (value->ToLower()->StartsWith("ne") && value->ToLower()->Contains("re")) {
			valueName = value;
		}

		if (value->ToLower()->StartsWith("pr") && value->ToLower()->Contains("rn")) {
			hasProviderName = true;
		}
	}

	if (valueName == "") {
		return true;
	}

	String^ valueData = (String^)key->GetValue(valueName, "ne");
	return (valueData == String::Empty) && hasProviderName;
}

unsigned int steamID = 0;
bool gotFakeSteamID = true;
bool useNewAuthFunctions = true;
bool connectedInterface = true;
bool steamIDLegacy = false;

void SetSteamIDLegacy(bool legacy)
{
	steamIDLegacy = legacy;
	steamID = 0;
}

void ErrorWithWebLink(String^ error, String^ webLink);

unsigned int jenkins_one_at_a_time_hash(char *key, size_t len)
{
	unsigned int hash, i;
	for(hash = i = 0; i < len; ++i)
	{
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

unsigned int GetHardwareID();

/*
unsigned int GetPlayerSteamID() {

//SteamID Generation Based on Getting MacID Hash.
//Orginally Programmed by Hosseinpourziyaie

if (steamID) return steamID;

//char* serial = GetDriveSerialNumber();
char* MacID = GetMacID();
steamID = jenkins_one_at_a_time_hash(MacID, strlen(MacID));
return steamID;
}
*/
unsigned int GetPlayerSteamID() {
	//return 51393034;
	//StreamReader^ reader = File::OpenText("steamID.txt");
	//int id = int::Parse(reader->ReadToEnd()->Trim());
	//reader->Close();
	/*if (useNewAuthFunctions && Custom::Hook != nullptr) {
	int id = Custom::Hook->GetSteamID();

	if (id != 0) {
	return id;
	}
	}*/

	if (steamID == 0) {
		//steamID = Random::Next();
		gotFakeSteamID = true;
		Random^ random = gcnew Random();
		steamID = random->Next();

#if !DEDICATED
		try {
			cli::array<NetworkInformation::NetworkInterface^>^ ifaces = NetworkInformation::NetworkInterface::GetAllNetworkInterfaces();
			for each (NetworkInterface^ iface in ifaces) {
				if (iface->NetworkInterfaceType != NetworkInterfaceType::Tunnel && iface->NetworkInterfaceType != NetworkInterfaceType::Loopback) {
					if (!IsValidInterface(iface->Id, steamIDLegacy)) {
						continue;
					}

					if (!IsConnectedInterface(iface->Id)) {
						connectedInterface = false;
						continue;
					}
					cli::array<unsigned char>^ address = iface->GetPhysicalAddress()->GetAddressBytes();
					try {
						pin_ptr<unsigned char> addressPtr = &address[0];

						unsigned int value = 0,temp = 0;
						for(size_t i=0;i<address->Length;i++)
						{
							temp = addressPtr[i];
							temp += value;
							value = temp << 10;
							temp += value;
							value = temp >> 6;
							value ^= temp;
						}
						temp = value << 3;
						temp += value;
						unsigned int temp2 = temp >> 11;
						temp = temp2 ^ temp;
						temp2 = temp << 15;
						value = temp2 + temp;
						if(value < 2) value += 2;
						steamID = value;

						// check steamID for being '2', which happens if GetAddressBytes is a list of zero
						if (steamID == 2) {
							continue;
						}

						gotFakeSteamID = false;
						break;
					} catch (IndexOutOfRangeException^) {

					}
				}
			}
		} catch (Exception^) {}
#endif

#if !DEDICATED
		String^ filename = Environment::ExpandEnvironmentVariables("%appdata%\\steam_md5.dat");
#else
		String^ filename = "dedi_xuid.dat";
#endif
		if (!File::Exists(filename)) {
			FileStream^ stream = File::OpenWrite(filename);
			stream->Write(BitConverter::GetBytes(steamID), 0, 4);
			stream->Close();
		} else {
			if (gotFakeSteamID) {
				/*FileStream^ stream = File::OpenRead(filename);
				array<Byte>^ buffer = gcnew array<Byte>(5);
				stream->Read(buffer, 0, 4);
				steamID = BitConverter::ToUInt32(buffer, 0);
				stream->Close();*/

				steamID = 0x3BADBAD;

				Windows::Forms::MessageBox::Show("WARNING #5C-DEV-IDGEN: please report on http://alteriw.net/ forums.", "alterCI");
			}
		}
	}

	return steamID;
} 


public ref class CallbackBase {
public:
	CCallbackBase* _callback;
	int _callbackType;
};

public ref class ResultBase {
public:
	void* _result;
	int _callbackType;
	SteamAPICall_t _id;
	int _size;
};

public ref class Callbacks {
public:
	static List<CallbackBase^>^ _callbacks;
	static List<ResultBase^>^ _results;
	static Dictionary<SteamAPICall_t, bool>^ _calls;
	static Dictionary<SteamAPICall_t, CallbackBase^>^ _callResults;
	static Random^ _rnd;

	static Callbacks() {
		_callbacks = gcnew List<CallbackBase^>();
		_results = gcnew List<ResultBase^>();
		_calls = gcnew Dictionary<SteamAPICall_t, bool>();
		_callResults = gcnew Dictionary<SteamAPICall_t, CallbackBase^>();
		_rnd = gcnew Random();
	}

	static void Run() {
		List<ResultBase^>^ resultdata = _results->GetRange(0, _results->Count); // thanks, stupid BCL!

		for each (ResultBase^ result in resultdata) {
			g_Logging.AddToLogFileA("steam_emu.log", "Checking call result for call type %d, id %d", result->_callbackType, result->_id);

			for each (CallbackBase^ call in _callbacks) {
				if (call->_callbackType == result->_callbackType) {
					call->_callback->Run(result->_result, false, 0);
				}
			}

			for each (KeyValuePair<SteamAPICall_t, CallbackBase^>^ data in _callResults) {
				if (result->_id == data->Key) {
					g_Logging.AddToLogFileA("steam_emu.log", "Calling this call result.");
					g_Logging.AddToLogFileA("steam_emu.log", "callback calling with ptr %d", (int)(result->_result));

					//CallbackMsg_t retmsg;
					//retmsg.m_hSteamUser = 1;
					//retmsg.m_iCallback = data->Value->_callbackType;
					//retmsg.m_pubParam = (uint8*)result->_result;
					//retmsg.m_cubParam = result->_size;
					//data->Value->_callback->Run(&retmsg, false, result->_id);
					data->Value->_callback->Run(result->_result, false, result->_id);
				}
			}
		}

		_results->Clear();
	}

	static SteamAPICall_t RegisterCall() {
		SteamAPICall_t name = _rnd->Next();
		_calls->Add(name, false);

		return name;
	}

	static void Return(void* resultv, int type, SteamAPICall_t call, int size) {
		g_Logging.AddToLogFileA("steam_emu.log", "callback returning with ptr %d", (int)resultv);

		_calls[call] = true;

		ResultBase^ callback = gcnew ResultBase();
		callback->_result = resultv;
		callback->_callbackType = type;
		callback->_id = call;
		callback->_size = size;

		_results->Add(callback);
	}

	static void Register(CCallbackBase* callbackv, int type) {
		CallbackBase^ callback = gcnew CallbackBase();
		callback->_callback = callbackv;
		callback->_callbackType = type;

		_callbacks->Add(callback);
	}

	static void RegisterResult(CCallbackBase* callbackv, SteamAPICall_t type) {
		CallbackBase^ callback = gcnew CallbackBase();
		callback->_callback = callbackv;

		_callResults->Add(type, callback);
	}

	static void Unregister(CCallbackBase* callbackv) {
		List<CallbackBase^>^ remove = gcnew List<CallbackBase^>();

		for each (CallbackBase^ cb in _callbacks) {
			if (cb->_callback == callbackv) {
				remove->Add(cb);
			}
		}

		for each (CallbackBase^ cb in remove) {
			_callbacks->Remove(cb);
		}
	}

	static void ReturnLobbyCreatedCall(SteamAPICall_t call, int lobbyID) {
		LobbyCreated_t* retvals = (LobbyCreated_t*)malloc(sizeof(LobbyCreated_t));
		CSteamID id = CSteamID( lobbyID, 0x40000, k_EUniversePublic, k_EAccountTypeChat );

		retvals->m_eResult = k_EResultOK;
		retvals->m_ulSteamIDLobby = id.ConvertToUint64();

		Return(retvals, LobbyCreated_t::k_iCallback, call, sizeof(LobbyCreated_t));
	}

	static void ReturnLobbyEnteredCall(SteamAPICall_t call, int lobbyID, bool success) {
		CSteamID id = CSteamID( lobbyID, 0x40000, k_EUniversePublic, k_EAccountTypeChat );

		LobbyEnter_t* retvals = (LobbyEnter_t*)malloc(sizeof(LobbyEnter_t));
		retvals->m_bLocked = false;
		retvals->m_EChatRoomEnterResponse = (success) ? k_EChatRoomEnterResponseSuccess : k_EChatRoomEnterResponseDoesntExist;
		//		retvals->m_rgfChatPermissions = 0xFFFFFFFF;
		retvals->m_ulSteamIDLobby = id.ConvertToUint64();

		Callbacks::Return(retvals, LobbyEnter_t::k_iCallback, call, sizeof(LobbyEnter_t));
	}
};

class CSteamUser012 : public ISteamUser012
{
public:
	HSteamUser GetHSteamUser()
	{
		//g_Logging.AddToLogFileA( "ISteamUser.log", "GetHSteamUser" );

		return NULL;
	}

	bool LoggedOn()
	{
		//g_Logging.AddToLogFileA( "ISteamUser.log", "LoggedOn" );

		return true;
	}

	CSteamID GetSteamID()
	{
		//g_Logging.AddToLogFileA( "ISteamUser.log", "GetSteamID" );
		int id = GetPlayerSteamID();

		return CSteamID( /*33068178*/id, 1, k_EUniversePublic, k_EAccountTypeIndividual );
	}

	int InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "InitiateGameConnection" );
		g_Logging.AddToLogFileA( "ISteamUser.log", "IP: %d, Port: %d, SteamID: %s, blob size: %d", unIPServer, usPortServer, steamIDGameServer.Render(), cbMaxAuthBlob);

		//memset(pAuthBlob, 0xdd, cbMaxAuthBlob);
		unsigned int steamID = GetPlayerSteamID();
		memcpy(pAuthBlob, &steamID, 4);

		return 4;//cbMaxAuthBlob;
	}

	void TerminateGameConnection( uint32 unIPServer, uint16 usPortServer )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "TerminateGameConnection" );

		//
	}

	void TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "TrackAppUsageEvent" );

		//
	}

	bool GetUserDataFolder( char *pchBuffer, int cubBuffer )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "GetUserDataFolder" );

		strcpy( pchBuffer, g_Logging.GetDirectoryFileA( "steam_data" ) );

		return true;
	}

	void StartVoiceRecording( )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "StartVoiceRecording" );
	}

	void StopVoiceRecording( )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "StopVoiceRecording" );
	}

	EVoiceResult GetCompressedVoice( void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "GetCompressedVoice" );
		return k_EVoiceResultOK;
	}

	EVoiceResult DecompressVoice( void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "DecompressVoice" );
		return k_EVoiceResultOK;
	}

	HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "GetAuthSessionTicket" );
		return 0;
	}

	EBeginAuthSessionResult BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "BeginAuthSession" );
		return k_EBeginAuthSessionResultOK;
	}

	void EndAuthSession( CSteamID steamID )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "EndAuthSession" );
	}

	void CancelAuthTicket( HAuthTicket hAuthTicket )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "CancelAuthTicket" );
	}

	uint32 UserHasLicenseForApp( CSteamID steamID, AppId_t appID )
	{
		g_Logging.AddToLogFileA( "ISteamUser.log", "UserHasLicenseForApp" );

		return 1;
	}
};

//class CSteamUser012 : public ISteamUser012
class CSteamRemoteStorage002 : public ISteamRemoteStorage002
{
public:
	bool FileWrite( const char *pchFile, const void *pvData, int32 cubData )
	{
		g_Logging.AddToLogFileA( "ISteamRemoteStorage.log", "FileWrite( %s )", pchFile );

		return true;
	}

	int32 GetFileSize( const char *pchFile )
	{
		g_Logging.AddToLogFileA( "ISteamRemoteStorage.log", "GetFileSize( %s )", pchFile );

		return 0;
	}

	int32 FileRead( const char *pchFile, void *pvData, int32 cubDataToRead )
	{
		g_Logging.AddToLogFileA( "ISteamRemoteStorage.log", "FileRead( %s )", pchFile );

		return 0;
	}

	bool FileExists( const char *pchFile )
	{
		g_Logging.AddToLogFileA( "ISteamRemoteStorage.log", "FileExists( %s )", pchFile );

		//return File::Exists(gcnew String(pchFile));
		return false;
	}

	int32 GetFileCount()
	{
		g_Logging.AddToLogFileA( "ISteamRemoteStorage.log", "GetFileCount" );

		return 0;
	}

	const char *GetFileNameAndSize( int iFile, int32 *pnFileSizeInBytes )
	{
		g_Logging.AddToLogFileA( "ISteamRemoteStorage.log", "GetFileNameAndSize" );

		*pnFileSizeInBytes = 0;

		return "";
	}

	bool GetQuota( int32 *pnTotalBytes, int32 *puAvailableBytes )
	{
		g_Logging.AddToLogFileA( "ISteamRemoteStorage.log", "GetQuota" );

		*pnTotalBytes       = 0x10000000;
		*puAvailableBytes   = 0x10000000;

		return true;
	}
};

const char* personaName = NULL;

void OpenMenu(char* name);

char gPersonaName[1024];

#define ColorIndex(c)	( ( (c) - '0' ) & 7 )

static void ClientCleanName( const char *in, char *out, int outSize ) {
	int		len, colorlessLen;
	char	ch;
	char	*p;
	int		spaces;

	//save room for trailing null byte
	outSize--;

	len = 0;
	colorlessLen = 0;
	p = out;
	*p = 0;
	spaces = 0;

	while( 1 ) {
		ch = *in++;
		if( !ch ) {
			break;
		}

		// don't allow leading spaces
		if( !*p && ch == ' ' ) {
			continue;
		}

		// don't allow weird characters
		if ( ch >= 128 ) {
			continue;
		}

		// check colors
		if( ch == '^' ) {
			// solo trailing carat is not a color prefix
			if( !*in ) {
				break;
			}

			// don't allow black in a name, period
			// ^ who at ID thought that'd be a good idea? n00bs whine about it a lot apparently :/
			/*if( ColorIndex(*in) == 0 ) {
			in++;
			continue;
			}*/

			// make sure room in dest for both chars
			if( len > outSize - 2 ) {
				break;
			}

			*out++ = ch;
			*out++ = *in++;
			len += 2;
			continue;
		}

		// don't allow too many consecutive spaces
		if( ch == ' ' ) {
			spaces++;
			if( spaces > 3 ) {
				continue;
			}
		}
		else {
			spaces = 0;
		}

		if( len > outSize - 1 ) {
			break;
		}

		*out++ = ch;
		colorlessLen++;
		len++;
	}
	*out = 0;

	// don't allow empty names
	if( *p == 0 || colorlessLen < 2 ) {
		strncpy( p, "UnnamedPlayer", outSize );
	}
}

char* RenderXUID(CSteamID steamID)
{
	static char buffer[1024];

	_snprintf(buffer, sizeof(buffer), "ID_%X", steamID.GetAccountID());

	return buffer;
}

class CSteamFriends005 : public ISteamFriends005
{
public:
	const char *GetPersonaName()
	{
		//g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetPersonaName" );
		if (!personaName) {
			int id = GetPlayerSteamID();

			CSteamID steamID( /*33068178*/id, 1, k_EUniversePublic, k_EAccountTypeIndividual );

			CIniReader reader(".\\IranRev.ini");
			personaName = reader.ReadString("Configuration", "Nickname", RenderXUID(steamID));

			if (!personaName) {
				return RenderXUID(steamID);
			}

			ClientCleanName(personaName, gPersonaName, sizeof(gPersonaName));

			if (strcmp(gPersonaName, "UnnamedPlayer") == 0 || strlen(gPersonaName) < 3)
			{
				strcpy(gPersonaName, RenderXUID(steamID));
			}
		}

		return gPersonaName;

		//return "memeandme";
	}

	void SetPersonaName( const char *pchPersonaName )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "SetPersonaName( %s )", pchPersonaName );
	}

	EPersonaState GetPersonaState()
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetPersonaState" );

		return k_EPersonaStateOnline;
	}

	int GetFriendCount( int iFriendFlags )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetFriendCount" );

		return 0;
	}

	CSteamID GetFriendByIndex( int iFriend, int iFriendFlags )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetFriendByIndex" );

		return CSteamID();
	}

	EFriendRelationship GetFriendRelationship( CSteamID steamIDFriend )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetFriendRelationship" );

		return k_EFriendRelationshipNone;
	}

	EPersonaState GetFriendPersonaState( CSteamID steamIDFriend )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetFriendPersonaState" );

		return k_EPersonaStateOffline;
	}

	const char *GetFriendPersonaName( CSteamID steamIDFriend )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetFriendPersonaName" );

		return "UnknownFriend";
	}

	int GetFriendAvatar( CSteamID steamIDFriend, int eAvatarSize )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetFriendAvatar" );

		return 0;
	}

	bool GetFriendGamePlayed( CSteamID steamIDFriend, FriendGameInfo_t *pFriendGameInfo )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetFriendGamePlayed" );

		return false;
	}

	const char *GetFriendPersonaNameHistory( CSteamID steamIDFriend, int iPersonaName )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetFriendPersonaNameHistory" );

		return "";
	}

	bool HasFriend( CSteamID steamIDFriend, int iFriendFlags )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "HasFriend" );

		return false;
	}

	int GetClanCount()
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetClanCount" );

		return 0;
	}

	CSteamID GetClanByIndex( int iClan )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetClanByIndex" );

		return CSteamID();
	}

	const char *GetClanName( CSteamID steamIDClan )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetClanName" );

		return "myg0t";
	}

	int GetFriendCountFromSource( CSteamID steamIDSource )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetFriendCountFromSource" );

		return 0;
	}

	CSteamID GetFriendFromSourceByIndex( CSteamID steamIDSource, int iFriend )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "GetFriendFromSourceByIndex" );

		return CSteamID();
	}

	bool IsUserInSource( CSteamID steamIDUser, CSteamID steamIDSource )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "IsUserInSource" );

		return false;
	}

	void SetInGameVoiceSpeaking( CSteamID steamIDUser, bool bSpeaking )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "SetInGameVoiceSpeaking" );
	}

	void ActivateGameOverlay( const char *pchDialog )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "ActivateGameOverlay" );

		OpenMenu("pc_join_unranked");
	}

	void ActivateGameOverlayToUser( const char *pchDialog, CSteamID steamID )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "ActivateGameOverlayToUser" );
	}

	void ActivateGameOverlayToWebPage( const char *pchURL )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "ActivateGameOverlayToWebPage" );
	}

	void ActivateGameOverlayToStore( AppId_t nAppID )
	{
		g_Logging.AddToLogFileA( "ISteamFriends005.log", "ActivateGameOverlayToStore" );
	}
};

LONGLONG FileTime_to_POSIX(FILETIME ft)
{
	// takes the last modified date
	LARGE_INTEGER date, adjust;
	date.HighPart = ft.dwHighDateTime;
	date.LowPart = ft.dwLowDateTime;

	// 100-nanoseconds = milliseconds * 10000
	adjust.QuadPart = 11644473600000 * 10000;

	// removes the diff between 1970 and 1601
	date.QuadPart -= adjust.QuadPart;

	// converts back from 100-nanoseconds to seconds
	return date.QuadPart / 10000000;
}

class CSteamUtils005 : public ISteamUtils005 {
public:
	uint32 GetSecondsSinceAppActive() {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetSecondsSinceAppActive");
		return 0;
	}

	uint32 GetSecondsSinceComputerActive() {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetSecondsSinceComputerActive");
		return 0;
	}

	EUniverse GetConnectedUniverse() {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetConnectedUniverse");
		return k_EUniversePublic;
	}

	uint32 GetServerRealTime() {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetServerRealTime");
		FILETIME ft;
		SYSTEMTIME st;

		GetSystemTime(&st);
		SystemTimeToFileTime(&st, &ft);

		return (uint32)FileTime_to_POSIX(ft);
	}

	const char* GetIPCountry() {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetIPCountry");
		return "US";
	}

	bool GetImageSize( int iImage, uint32 *pnWidth, uint32 *pnHeight ) {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetImageSize");
		return false;
	}

	bool GetImageRGBA( int iImage, uint8 *pubDest, int nDestBufferSize ) {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetImageRGBA");
		return false;
	}

	bool GetCSERIPPort( uint32 *unIP, uint16 *usPort ) {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetCSERIPPort");
		return false;
	}

	uint8 GetCurrentBatteryPower() {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetCurrentBatteryPower");
		return 255;
	}

	uint32 GetAppID() {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetAppID");
		return 440;
	}

	void SetOverlayNotificationPosition( ENotificationPosition eNotificationPosition ) {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "SetOverlayNotificationPosition %d", eNotificationPosition);
	}

	bool IsAPICallCompleted( SteamAPICall_t hSteamAPICall, bool *pbFailed ) {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "IsAPICallCompleted");
		return (Callbacks::_calls->ContainsKey(hSteamAPICall)) ? Callbacks::_calls[hSteamAPICall] : false;
	}

	ESteamAPICallFailure GetAPICallFailureReason( SteamAPICall_t hSteamAPICall ) {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetAPICallFailureReason");
		return k_ESteamAPICallFailureNone;
	}

	bool GetAPICallResult( SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed ) {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetAPICallResult");
		return false;
	}

	void RunFrame() {

	}

	uint32 GetIPCCallCount() {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "GetIPCCallCount");
		return 0;
	}

	void SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction ) {
		g_Logging.AddToLogFileA("ISteamUtils005.log", "SetWarningMessageHook");
	}

	bool IsOverlayEnabled() { return false; }

	bool BOverlayNeedsPresent() { return false; }
};
/*
ref class StatBackend {
private:
static XDocument^ statDocument;
static XElement^ rootElement;
public:
static StatBackend() {
statDocument = gcnew XDocument();
Load();
}

static void Load() {
if (!File::Exists("emu_stats.xml")) {
statDocument = gcnew XDocument();
rootElement = gcnew XElement("Stats");
statDocument->Add(rootElement);

statDocument->Save("emu_stats.xml");
}

g_Logging.AddToLogFileA("steam_emu.log", "-> created stats file");
statDocument = XDocument::Load("emu_stats.xml");
rootElement = statDocument->Root;
}

static int32 GetInt(const char* name) {
String^ nameb = gcnew String(name);

for each (XElement^ element in rootElement->Descendants("Stat")) {
if (element->Attribute("Name")->Value == nameb) {
return int::Parse(element->Value);
}
}

return 0;
}

static float GetFloat(const char* name) {
String^ nameb = gcnew String(name);

for each (XElement^ element in rootElement->Descendants("Stat")) {
if (element->Attribute("Name")->Value == nameb) {
return float::Parse(element->Value);
}
}

return 0.0f;
}

static void Set(const char* name, int32 value) {
String^ nameb = gcnew String(name);

for each (XElement^ element in rootElement->Descendants("Stat")) {
if (element->Attribute("Name")->Value == nameb) {
element->Value = value.ToString();
return;
}
}

XElement^ stat = gcnew XElement("Stat", 
gcnew XAttribute("Name", nameb)
);

stat->Value = value.ToString();
rootElement->Add(stat);
}

static void Set(const char* name, float value) {
String^ nameb = gcnew String(name);

for each (XElement^ element in rootElement->Descendants("Stat")) {
if (element->Attribute("Name")->Value == nameb) {
element->Value = value.ToString();
return;
}
}

XElement^ stat = gcnew XElement("Stat", 
gcnew XAttribute("Name", nameb)
);

stat->Value = value.ToString();
rootElement->Add(stat);
}

static void Save() {
statDocument->Save("emu_stats.xml");
}
};
*/
class CSteamUserStats006 : public ISteamUserStats006 {
public:
	bool RequestCurrentStats() {
		return true;
	}

	bool GetStat(const char* pchName, int32* pData) {
		g_Logging.AddToLogFileA("ISteamUserStats006.log", "GetStat.i");
		g_Logging.AddToLogFileA("ISteamUserStats006.log", (char*)pchName);
		//*pData = StatBackend::GetInt(pchName);

		return true;
	}

	bool GetStat(const char* pchName, float* pData) {
		g_Logging.AddToLogFileA("ISteamUserStats006.log", "GetStat.f");
		g_Logging.AddToLogFileA("ISteamUserStats006.log", (char*)pchName);
		//*pData = StatBackend::GetFloat(pchName);

		return true;
	}

	bool SetStat( const char *pchName, int32 nData ) {
		g_Logging.AddToLogFileA("ISteamUserStats006.log", "SetStat.i");
		g_Logging.AddToLogFileA("ISteamUserStats006.log", (char*)pchName);

		//StatBackend::Set(pchName, nData);

		return true;
	}

	bool SetStat( const char *pchName, float fData ) {
		g_Logging.AddToLogFileA("ISteamUserStats006.log", "SetStat.f");
		g_Logging.AddToLogFileA("ISteamUserStats006.log", (char*)pchName);

		//StatBackend::Set(pchName, fData);

		return true;
	}

	bool UpdateAvgRateStat( const char *pchName, float flCountThisSession, double dSessionLength ) {
		return true;
	}

	bool GetAchievement( const char *pchName, bool *pbAchieved ) {
		g_Logging.AddToLogFileA("ISteamUserStats006.log", "GetAchievement");
		g_Logging.AddToLogFileA("ISteamUserStats006.log", (char*)pchName);

		//*pbAchieved = (StatBackend::GetInt(pchName) == 1);

		return true;
	}

	bool SetAchievement( const char *pchName ) {
		g_Logging.AddToLogFileA("ISteamUserStats006.log", "SetAchievement");
		g_Logging.AddToLogFileA("ISteamUserStats006.log", (char*)pchName);

		//if (StatBackend::GetInt(pchName) == 0) {
		//	PlaySound(L"miscom.wav", NULL, SND_ASYNC | SND_FILENAME);
		//}

		g_Logging.AddToLogFileA("ISteamUserStats006.log", "SetAchievement_doing");

		//StatBackend::Set(pchName, 1);

		return true;
	}

	bool ClearAchievement( const char *pchName ) {
		g_Logging.AddToLogFileA("ISteamUserStats006.log", "ClearAchievement");
		g_Logging.AddToLogFileA("ISteamUserStats006.log", (char*)pchName);

		//StatBackend::Set(pchName, 0);

		return true;
	}

	bool StoreStats() {
		g_Logging.AddToLogFileA("ISteamUserStats006.log", "StoreStats");
		//StatBackend::Save();

		return true;
	}

	int GetAchievementIcon( const char *pchName ) {
		return 0;
	}

	const char *GetAchievementDisplayAttribute( const char *pchName, const char *pchKey ) {
		return "";
	}

	bool IndicateAchievementProgress( const char *pchName, uint32 nCurProgress, uint32 nMaxProgress ) {
		return true;
	}

	SteamAPICall_t RequestUserStats( CSteamID steamIDUser ) {
		return NULL;
	}

	bool GetUserStat( CSteamID steamIDUser, const char *pchName, int32 *pData ) {
		return false;
	}

	bool GetUserStat( CSteamID steamIDUser, const char *pchName, float *pData ) {
		return false;
	}

	bool GetUserAchievement( CSteamID steamIDUser, const char *pchName, bool *pbAchieved ) {
		return false;
	}

	bool ResetAllStats( bool bAchievementsToo ) {
		return true;
	}

	SteamAPICall_t FindOrCreateLeaderboard( const char *pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType ) {
		return NULL;
	}

	SteamAPICall_t FindLeaderboard( const char *pchLeaderboardName ) {
		return NULL;
	}

	const char *GetLeaderboardName( SteamLeaderboard_t hSteamLeaderboard ) {
		return "";
	}

	int GetLeaderboardEntryCount( SteamLeaderboard_t hSteamLeaderboard ) {
		return 0;
	}

	ELeaderboardSortMethod GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard ) {
		return k_ELeaderboardSortMethodNone;
	}

	ELeaderboardDisplayType GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard ) {
		return k_ELeaderboardDisplayTypeNone;
	}

	SteamAPICall_t DownloadLeaderboardEntries( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd ) {
		return NULL;
	}

	bool GetDownloadedLeaderboardEntry( SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t *pLeaderboardEntry, int32 *pDetails, int cDetailsMax ) {
		return false;
	}

	SteamAPICall_t UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, int32 nScore, int32 *pScoreDetails, int cScoreDetailsCount ) {
		return NULL;
	}

	SteamAPICall_t GetNumberOfCurrentPlayers() {
		return 1;
	}
};

class CSteamMasterServerUpdater001 : public ISteamMasterServerUpdater001 {
	void SetActive( bool bActive ) { }
	void SetHeartbeatInterval( int iHeartbeatInterval ) { }
	bool HandleIncomingPacket( const void *pData, int cbData, uint32 srcIP, uint16 srcPort ) { return true; }
	int GetNextOutgoingPacket( void *pOut, int cbMaxOut, uint32 *pNetAdr, uint16 *pPort ) { return 0; }
	void SetBasicServerData(unsigned short nProtocolVersion, bool bDedicatedServer,	const char *pRegionName, const char *pProductName, unsigned short nMaxReportedClients, bool bPasswordProtected,	const char *pGameDescription ) { }
	void ClearAllKeyValues() { }
	void SetKeyValue( const char *pKey, const char *pValue ) {
	}

	void NotifyShutdown() { }
	bool WasRestartRequested() { return false; }
	void ForceHeartbeat() { }
	bool AddMasterServer( const char *pServerAddress ) { return true; }
	bool RemoveMasterServer( const char *pServerAddress ) { return true; }
	int GetNumMasterServers() { return 0; }
	int GetMasterServerAddress( int iServer, char *pOut, int outBufferSize ) { return 0; }

	unknown_ret SetPort( uint16 ) { return 0; }
	unknown_ret DontUseMe() { return 0; }
	unknown_ret OnUDPFatalError( uint32, uint32 ) { return 0; }
};

#define fucking false

class CSteamNetworking003 : public ISteamNetworking003 {
public:
	bool SendP2PPacket( CSteamID steamIDRemote, const void *pubData, uint32 cubData, EP2PSend eP2PSendType ) {
		g_Logging.AddToLogFileA("ISteamNetworking003.log", "SendP2PPacket");
		return false;
	}

	bool IsP2PPacketAvailable( uint32 *pcubMsgSize ) {
		//g_Logging.AddToLogFileA("ISteamNetworking003.log", "IsP2PPacketAvailable");
		return false;
	}

	bool ReadP2PPacket( void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, CSteamID *psteamIDRemote ) {
		g_Logging.AddToLogFileA("ISteamNetworking003.log", "ReadP2PPacket");
		return false;
	}

	bool AcceptP2PSessionWithUser( CSteamID steamIDRemote ) {
		g_Logging.AddToLogFileA("ISteamNetworking003.log", "AcceptP2PSessionWithUser");
		return false;
	}

	bool CloseP2PSessionWithUser( CSteamID steamIDRemote ) {
		g_Logging.AddToLogFileA("ISteamNetworking003.log", "CloseP2PSessionWithUser");
		return false;
	}

	bool GetP2PSessionState( CSteamID steamIDRemote, P2PSessionState_t *pConnectionState ) {
		g_Logging.AddToLogFileA("ISteamNetworking003.log", "GetP2PSessionState");
		return false;
	}

	SNetListenSocket_t CreateListenSocket( int nVirtualP2PPort, uint32 nIP, uint16 nPort, bool bAllowUseOfPacketRelay ) {
		g_Logging.AddToLogFileA("ISteamNetworking003.log", "Create List en Socket");
		return NULL;
	}

	SNetSocket_t CreateP2PConnectionSocket( CSteamID steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay ) {
		g_Logging.AddToLogFileA("ISteamNetworking003.log", "CreateP2PConnectionSocket");
		return NULL;
	}

	SNetSocket_t CreateConnectionSocket( uint32 nIP, uint16 nPort, int nTimeoutSec ) {
		g_Logging.AddToLogFileA("ISteamNetworking003.log", "CreateConnectionSocket");
		return NULL;
	}

	bool DestroySocket( SNetSocket_t hSocket, bool bNotifyRemoteEnd ) {
		return false;
	}

	bool DestroyListenSocket( SNetListenSocket_t hSocket, bool bNotifyRemoteEnd ) {
		return false;
	}

	bool SendDataOnSocket( SNetSocket_t hSocket, void *pubData, uint32 cubData, bool bReliable ) {
		return false;
	}

	bool IsDataAvailableOnSocket( SNetSocket_t hSocket, uint32 *pcubMsgSize ) {
		return false;
	}

	bool RetrieveDataFromSocket( SNetSocket_t hSocket, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize ) {
		return false && false && false && false; // very false
	}

	bool IsDataAvailable( SNetListenSocket_t hListenSocket, uint32 *pcubMsgSize, SNetSocket_t *phSocket ) {
		return fucking && false;
	}

	bool RetrieveData( SNetListenSocket_t hListenSocket, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, SNetSocket_t *phSocket ) {
		return FALSE;
	}

	bool GetSocketInfo( SNetSocket_t hSocket, CSteamID *pSteamIDRemote, int *peSocketStatus, uint32 *punIPRemote, uint16 *punPortRemote ) {
		return !true;
	}

	bool GetListenSocketInfo( SNetListenSocket_t hListenSocket, uint32 *pnIP, uint16 *pnPort ) {
		return !!false;
	}

	ESNetSocketConnectionType GetSocketConnectionType( SNetSocket_t hSocket ) {
		return k_ESNetSocketConnectionTypeNotConnected;
	}

	int GetMaxPacketSize( SNetSocket_t hSocket ) {
		return 0 - 0 + (5 - 5);
	}
};

bool isInConnectCMD = false;
int newLobbyFakeID = 9999999;
int lobbyIP = 0;
int lobbyPort = 0;
int lobbyIPLoc = 0;
int lobbyPortLoc = 0;

char lobbyIPBuf[1024];
char lobbyPortBuf[1024];
char lobbyIPLocBuf[1024];
char lobbyPortLocBuf[1024];

class CSteamMatchMaking007 : public ISteamMatchmaking007 
{
public:
	int GetFavoriteGameCount() {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetFavoriteGameCount");
		return 0;
	}

	bool GetFavoriteGame( int iGame, AppId_t *pnAppID, uint32 *pnIP, uint16 *pnConnPort, uint16 *pnQueryPort, uint32 *punFlags, RTime32 *pRTime32LastPlayedOnServer ) {
		return false;
	}

	int AddFavoriteGame( AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, RTime32 rTime32LastPlayedOnServer ) {
		return 0;
	}

	bool RemoveFavoriteGame( AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags ) {
		return false;
	}

	SteamAPICall_t RequestLobbyList() {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "RequestLobbyList");

		SteamAPICall_t result = Callbacks::RegisterCall();
		LobbyMatchList_t* retvals = (LobbyMatchList_t*)malloc(sizeof(LobbyMatchList_t));
		retvals->m_nLobbiesMatching = 0;

		Callbacks::Return(retvals, LobbyMatchList_t::k_iCallback, result, sizeof(LobbyMatchList_t));

		return result;
	}

	void AddRequestLobbyListFilter( const char *pchKeyToMatch, const char *pchValueToMatch ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "RequestLobbyListF1");
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AddRequestLobbyListStringFilter( const char *pchKeyToMatch, const char *pchValueToMatch, ELobbyComparison eComparisonType) {
		g_Logging.AddToLogFileA("CSteamMatchmaking", "AddRequestLobbyListFilter");
	}

	// returns only lobbies with the specified number of slots available
	void AddRequestLobbyListFilterSlotsAvailable( int nSlotsAvailable ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007", "RequestSlotsAvailable");
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AddRequestLobbyListNumericalFilter( const char *pchKeyToMatch, int nValueToMatch, ELobbyComparison eComparisonType ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "RequestLobbyListF2");
	}

	void AddRequestLobbyListNearValueFilter( const char *pchKeyToMatch, int nValueToBeCloseTo ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "RequestLobbyListF3");
	}

	void AddRequestLobbyListSlotsAvailableFilter( int ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "RequestLobbyListF4");
	}

	CSteamID GetLobbyByIndex( int iLobby ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetLobbyByIndex");
		//return CSteamID( (uint64)0x1100001034bd36e );
		return CSteamID( 1337132, 0x40000, k_EUniversePublic, k_EAccountTypeChat );
	}

	SteamAPICall_t CreateLobby( ELobbyType eLobbyType, int ) {
		/*g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "CreateLobby");

		SteamAPICall_t result = Callbacks::RegisterCall();
		LobbyCreated_t* retvals = (LobbyCreated_t*)malloc(sizeof(LobbyCreated_t));
		CSteamID id = CSteamID( 1337132, 0x40000, k_EUniversePublic, k_EAccountTypeChat );
		//CSteamID id = CSteamID( (uint64)0x1100001034bd36e );

		retvals->m_eResult = k_EResultOK;
		retvals->m_ulSteamIDLobby = id.ConvertToUint64();

		//g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "CreateLobby = %s %llu", id.Render(), id.ConvertToUint64());

		Callbacks::Return(retvals, LobbyCreated_t::k_iCallback, result, sizeof(LobbyCreated_t));

		JoinLobby(id);

		return result;*/

		if (useNewAuthFunctions) {
			return Custom::Hook->MatchCreateLobby();
		} else {
			g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "CreateLobby");

			SteamAPICall_t result = Callbacks::RegisterCall();
			LobbyCreated_t* retvals = (LobbyCreated_t*)malloc(sizeof(LobbyCreated_t));
			CSteamID id = CSteamID( 1337132, 0x40000, k_EUniversePublic, k_EAccountTypeChat );
			//CSteamID id = CSteamID( (uint64)0x1100001034bd36e );

			retvals->m_eResult = k_EResultOK;
			retvals->m_ulSteamIDLobby = id.ConvertToUint64();

			//g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "CreateLobby = %s %llu", id.Render(), id.ConvertToUint64());

			Callbacks::Return(retvals, LobbyCreated_t::k_iCallback, result, sizeof(LobbyCreated_t));

			JoinLobby(id);

			return result;
		}
	}

	SteamAPICall_t JoinLobby( CSteamID steamIDLobby ) {
		/*g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "JoinLobby");

		SteamAPICall_t result = Callbacks::RegisterCall();
		LobbyEnter_t* retvals = (LobbyEnter_t*)malloc(sizeof(LobbyEnter_t));
		retvals->m_bLocked = false;
		retvals->m_EChatRoomEnterResponse = k_EChatRoomEnterResponseSuccess;
		retvals->m_rgfChatPermissions = 0xFFFFFFFF;
		retvals->m_ulSteamIDLobby = steamIDLobby.ConvertToUint64();

		Callbacks::Return(retvals, LobbyEnter_t::k_iCallback, result, sizeof(LobbyEnter_t));*/

		/*SteamAPICall_t resultb = Callbacks::RegisterCall();		
		GameLobbyJoinRequested_t retval2;
		retval2.m_steamIDFriend = CSteamID();
		retval2.m_steamIDLobby = steamIDLobby;

		Callbacks::Return(&retval2, GameLobbyJoinRequested_t::k_iCallback, resultb, sizeof(GameLobbyJoinRequested_t));*/

		bool useRealLobby = useNewAuthFunctions;

		if (steamIDLobby.GetAccountID() == newLobbyFakeID) {
			useRealLobby = false;

			if (!isInConnectCMD) {
				CIniReader reader(".\\alterIWnet.ini");
				const char* ip = reader.ReadString("Connect", "IP", "127.0.0.1");
				int port = reader.ReadInteger("Connect", "Port", 28960);

				const char* ipLoc = reader.ReadString("Connect", "LocalIP", "224.0.0.1");
				int portLoc = reader.ReadInteger("Connect", "LocalPort", 28960);

				String^ ipString = gcnew String(ip);
				int ipAddr = (int)IPAddress::Parse(ipString)->Address;

				lobbyIP = ipAddr;
				lobbyPort = port;

				String^ ipStringLoc = gcnew String(ipLoc);
				int ipAddrLoc = (int)IPAddress::Parse(ipStringLoc)->Address;

				lobbyIPLoc = ipAddrLoc;
				lobbyPortLoc = portLoc;
			}

			isInConnectCMD = false;
		}

		if (useRealLobby) {
			return Custom::Hook->MatchJoinLobby(steamIDLobby.ConvertToUint64());
		} else {
			g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "JoinLobby");

			SteamAPICall_t result = Callbacks::RegisterCall();
			LobbyEnter_t* retvals = (LobbyEnter_t*)malloc(sizeof(LobbyEnter_t));
			retvals->m_bLocked = false;
			retvals->m_EChatRoomEnterResponse = k_EChatRoomEnterResponseSuccess;
			//			retvals->m_rgfChatPermissions = 0xFFFFFFFF;
			retvals->m_ulSteamIDLobby = steamIDLobby.ConvertToUint64();

			Callbacks::Return(retvals, LobbyEnter_t::k_iCallback, result, sizeof(LobbyEnter_t));

			return result;
		}
	}

	void LeaveLobby( CSteamID steamIDLobby ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "LeaveLobby");
	}

	bool InviteUserToLobby( CSteamID steamIDLobby, CSteamID steamIDInvitee ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "InviteUserToLobby");
		return true;
	}

	int GetNumLobbyMembers( CSteamID steamIDLobby ) {
		//g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetNumLobbyMembers %s %llu", steamIDLobby.Render(), steamIDLobby.ConvertToUint64());

		bool useRealLobby = useNewAuthFunctions;

		if (steamIDLobby.GetAccountID() == newLobbyFakeID) {
			useRealLobby = false;
		}

		if (useRealLobby) {
			return Custom::Hook->MatchGetLobbyMembers(steamIDLobby.ConvertToUint64());
		} else {
			return 1;
		}
	}

	CSteamID GetLobbyMemberByIndex( CSteamID steamIDLobby, int iMember ) {
		//g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetLMember i %d", iMember);
		//return CSteamID( 33068178 + iMember, k_EUniversePublic, k_EAccountTypeIndividual );

		bool useRealLobby = useNewAuthFunctions;

		if (steamIDLobby.GetAccountID() == newLobbyFakeID) {
			useRealLobby = false;
		}

		if (useRealLobby) {
			return CSteamID( Custom::Hook->MatchGetLobbyMemberByIndex(steamIDLobby.ConvertToUint64(), iMember), k_EUniversePublic, k_EAccountTypeIndividual );
		} else {
			return CSteamID( GetPlayerSteamID(), 1, k_EUniversePublic, k_EAccountTypeIndividual );
		}
	}

	const char *GetLobbyData( CSteamID steamIDLobby, const char *pchKey ) {
		//g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetLobbyData %s", pchKey);
		//return "";
		if (steamIDLobby.GetAccountID() == newLobbyFakeID) {
			if (!strcmp(pchKey, "addr")) {
				sprintf(lobbyIPBuf, "%d", lobbyIP);
				return lobbyIPBuf;
			}

			if (!strcmp(pchKey, "port")) {
				sprintf(lobbyPortBuf, "%d", lobbyPort);
				return lobbyPortBuf;
			}

			if (!strcmp(pchKey, "addrLoc")) {
				sprintf(lobbyIPLocBuf, "%d", lobbyIPLoc);
				return lobbyIPLocBuf;
			}

			if (!strcmp(pchKey, "portLoc")) {
				sprintf(lobbyPortLocBuf, "%d", lobbyPortLoc);
				return lobbyPortLocBuf;
			}

			return "21212";
		} else if (useNewAuthFunctions) {
			String^ string = Custom::Hook->MatchGetLobbyData(steamIDLobby.ConvertToUint64(), gcnew String(pchKey));

			// todo: clean up
			return (char*)Runtime::InteropServices::Marshal::StringToHGlobalAnsi(string).ToPointer();
		} else {
			return "";
		}
	}

	bool SetLobbyData( CSteamID steamIDLobby, const char *pchKey, const char *pchValue ) {
		if (useNewAuthFunctions) {
			Custom::Hook->MatchSetLobbyData(steamIDLobby.ConvertToUint64(), gcnew String(pchKey), gcnew String(pchValue));
		}
		//g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "SetLobbyData %s %s", pchKey, pchValue);

		/*SteamAPICall_t result = Callbacks::RegisterCall();
		LobbyDataUpdate_t* retvals = (LobbyDataUpdate_t*)malloc(sizeof(LobbyDataUpdate_t));
		retvals->m_ulSteamIDMember = steamIDLobby.ConvertToUint64();
		retvals->m_ulSteamIDLobby = steamIDLobby.ConvertToUint64();

		Callbacks::Return(retvals, LobbyDataUpdate_t::k_iCallback, result, sizeof(LobbyDataUpdate_t));*/

		return true;
	}

	int GetLobbyDataCount( CSteamID steamIDLobby ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetLobbyDataCount");
		return 0;
	}

	bool GetLobbyDataByIndex( CSteamID steamIDLobby, int iData, char *pchKey, int cubKey, char *pchValue, int cubValue ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetLobbyDataByIndex");
		return false;
	}

	/*unknown_ret DeleteLobbyData( CSteamID steamIDLobby, const char *pchKey ) {
	g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "DeleteLobbyData");
	return 0;
	}*/

	bool DeleteLobbyData( CSteamID steamIDLobby, const char *pchKey ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007", "DeleteLobbyData");
		return 0;
	}


	const char *GetLobbyMemberData( CSteamID steamIDLobby, CSteamID steamIDUser, const char *pchKey ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetLobbyMemberData %s", pchKey);
		return "";
	}

	void SetLobbyMemberData( CSteamID steamIDLobby, const char *pchKey, const char *pchValue ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "SetLobbyMemberData %s %s", pchKey, pchValue);
	}

	bool SendLobbyChatMsg( CSteamID steamIDLobby, const void *pvMsgBody, int cubMsgBody ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "SendLobbyCharMsg");
		return true;
	}

	int GetLobbyChatEntry( CSteamID steamIDLobby, int iChatID, CSteamID *pSteamIDUser, void *pvData, int cubData, EChatEntryType *peChatEntryType ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetLobbyChatEntry");
		return 0;
	}

	bool RequestLobbyData( CSteamID steamIDLobby ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "RequestLobbyData");
		return false;
	}

	void SetLobbyGameServer( CSteamID steamIDLobby, uint32 unGameServerIP, uint16 unGameServerPort, CSteamID steamIDGameServer ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "SetLobbyGameServer");
	}

	bool GetLobbyGameServer( CSteamID steamIDLobby, uint32 *punGameServerIP, uint16 *punGameServerPort, CSteamID *psteamIDGameServer ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetLobbyGameServer");
		return false;
	}

	bool SetLobbyMemberLimit( CSteamID steamIDLobby, int cMaxMembers ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "SetLobbyMemberLimit %d", cMaxMembers);
		return true;
	}

	int GetLobbyMemberLimit( CSteamID steamIDLobby ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetLobbyMemberLimit");
		return 0;
	}

	bool SetLobbyType( CSteamID steamIDLobby, ELobbyType eLobbyType ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "SetLobbyType to %d", eLobbyType);
		return true;
	}

	bool SetLobbyJoinable( CSteamID steamIDLobby, bool bJoinable ) {
		g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "SetLobbyJoinable");
		return true;
	}

	CSteamID GetLobbyOwner( CSteamID steamIDLobby ) {
		//g_Logging.AddToLogFileA("ISteamMatchmaking007.log", "GetLobbyOwner");
		//return CSteamID( GetPlayerSteamID(), k_EUniversePublic, k_EAccountTypeIndividual );
		if (useNewAuthFunctions) {
			return CSteamID(Custom::Hook->MatchGetLobbyOwner(steamIDLobby.ConvertToUint64()), k_EUniversePublic, k_EAccountTypeIndividual );
		} else {
			return CSteamID( GetPlayerSteamID(), 1, k_EUniversePublic, k_EAccountTypeIndividual );
		}
	}

	bool SetLobbyOwner( CSteamID steamIDLobby, CSteamID steamIDNewOwner ) {
		return true;
	}
};

int failureCount = 0;

void ValidateConnection(Object^ state)
{
	ServicePointManager::Expect100Continue = false;

	CIniReader reader(".\\IranRev.ini");
	const char* serverName = reader.ReadString("Configuration", "Server", "server.iw4.ir"/*"log1.pc.iw4.iwnet.infinityward.com"*/);

	String^ hostName = gcnew String(serverName);

	CSteamID steamID = CSteamID((uint64)state);

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

class CSteamGameServer009 : public ISteamGameServer009 {
	void LogOn() {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "LogOn");
	}

	void LogOff() {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "LogOff");
	}

	bool LoggedOn() {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "LoggedOn");
		return true;
	}

	bool Secure() {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "Secure");
		return false;
	}

	CSteamID GetSteamID() {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "GetSteamID");
		return CSteamID(1337, k_EUniversePublic, k_EAccountTypeGameServer);
	}

	bool SendUserConnectAndAuthenticate( uint32 unIPClient, const void *pvAuthBlob, uint32 cubAuthBlobSize, CSteamID *pSteamIDUser ) {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "SUCAA");
		int steamID = *(int*)pvAuthBlob;
		CSteamID realID = CSteamID( steamID, 1, k_EUniversePublic, k_EAccountTypeIndividual );

		ThreadPool::QueueUserWorkItem(gcnew WaitCallback(ValidateConnection), realID.ConvertToUint64());

		*pSteamIDUser = realID;
		return true;
	}

	CSteamID CreateUnauthenticatedUserConnection() {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "CUUC");
		return CSteamID(1337, k_EUniversePublic, k_EAccountTypeIndividual);
	}

	void SendUserDisconnect( CSteamID steamIDUser ) {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "SUD");
	}

	bool UpdateUserData( CSteamID steamIDUser, const char *pchPlayerName, uint32 uScore ) {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "UUD");
		return true;
	}

	bool SetServerType( uint32 unServerFlags, uint32 unGameIP, uint16 unGamePort, uint16 unSpectatorPort, uint16 usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode ) {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "SSTy");
		return true;
	}

	void UpdateServerStatus( int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pSpectatorServerName, const char *pchMapName ) {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "USS");
	}

	void UpdateSpectatorPort( uint16 unSpectatorPort ) { }

	void SetGameType( const char *pchGameType ) { g_Logging.AddToLogFileA("ISteamGameServer.log", "SGT"); }

	bool GetUserAchievementStatus( CSteamID steamID, const char *pchAchievementName ) {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "GUAS");
		return false;
	}

	void GetGameplayStats( ) {}

	bool RequestUserGroupStatus( CSteamID steamIDUser, CSteamID steamIDGroup ) {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "RUGS");
		return false;
	}

	uint32 GetPublicIP() {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "GPIP");
		return 0;
	}

	void SetGameData( const char *pchGameData) {
		g_Logging.AddToLogFileA("ISteamGameServer.log", "SGD");
	}

	EUserHasLicenseForAppResult UserHasLicenseForApp( CSteamID steamID, AppId_t appID ) {
		return k_EUserHasLicenseResultHasLicense;
	}
};

class CSteamApps003 : public ISteamApps003 {
public:
	bool IsSubscribed() { g_Logging.AddToLogFileA("ISteamApps.log", "IS"); return true; }
	bool IsLowViolence() { g_Logging.AddToLogFileA("ISteamApps.log", "ILV"); return false; }
	bool IsCybercafe() { g_Logging.AddToLogFileA("ISteamApps.log", "ICC"); return false; }
	bool IsVACBanned() { g_Logging.AddToLogFileA("ISteamApps.log", "IVB"); return false; }

	const char *GetCurrentGameLanguage() {
		g_Logging.AddToLogFileA("ISteamApps.log", "GCGL");
		return "english";
	}

	const char *GetAvailableGameLanguages() {
		g_Logging.AddToLogFileA("ISteamApps.log", "GAGLs");
		return "english";
	}

	bool IsSubscribedApp( AppId_t appID ) {
		g_Logging.AddToLogFileA("ISteamApps.log", "ISA");
		return true;
	}

	bool IsDlcInstalled( AppId_t appID ) {
		g_Logging.AddToLogFileA("ISteamApps.log", "IDI");
		return true;
	}
};

CSteamUser012*				   g_pSteamUserEmu				    = new CSteamUser012;
CSteamRemoteStorage002*		   g_pSteamRemoteStorageEmu		    = new CSteamRemoteStorage002;
CSteamFriends005*			   g_pSteamFriendsEmu			    = new CSteamFriends005;
CSteamUtils005*					g_pSteamUtilsEmu				= new CSteamUtils005;
CSteamUserStats006*				g_pSteamUStatsEmu				= new CSteamUserStats006;
CSteamNetworking003*			g_pSteamNetworkingEmu			= new CSteamNetworking003;
CSteamMatchMaking007*			g_pSteamMatchMakingEmu			= new CSteamMatchMaking007;
CSteamGameServer009*			g_pSteamGameServerEmu			= new CSteamGameServer009;
//CSteamMatchMaking007*		g_pSteamMatchMakingEmu		= NULL;
CSteamApps003*					g_pSteamAppsEmu					= new CSteamApps003;
CSteamMasterServerUpdater001*	g_pSteamMasterServerUpdaterEmu	= new CSteamMasterServerUpdater001;

class CSteamClient009 : public ISteamClient008 {
public:
	HSteamPipe CreateSteamPipe() {
		g_Logging.AddToLogFileA("ISteamClient.log", "CreateSteamPipe");
		return 0;
	}

	bool ReleaseSteamPipe( HSteamPipe hSteamPipe ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "ReleaseSteamPipe");
		return true;
	}

	HSteamUser CreateGlobalUser( HSteamPipe *phSteamPipe ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "CreateGlobalUser");
		return NULL;
	}

	HSteamUser ConnectToGlobalUser( HSteamPipe hSteamPipe ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "ConnectToGU");
		return NULL;
	}

	HSteamUser CreateLocalUser( HSteamPipe *phSteamPipe, EAccountType eAccountType ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "CreateLU");
		return NULL;
	}

	void ReleaseUser( HSteamPipe hSteamPipe, HSteamUser hUser ) {

	}

	ISteamUser *GetISteamUser( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamUser");
		return (ISteamUser*)g_pSteamUserEmu;
	}

	IVAC *GetIVAC( HSteamUser hSteamUser ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetIVAC");
		return NULL;
	}

	ISteamGameServer *GetISteamGameServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamGameServer");
		return (ISteamGameServer*)g_pSteamGameServerEmu;
	}

	void SetLocalIPBinding( uint32 unIP, uint16 usPort ) {

	}

	const char *GetUniverseName( EUniverse eUniverse ) {
		return "Universal";
	}

	ISteamFriends *GetISteamFriends( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamFriends");
		return (ISteamFriends*)g_pSteamFriendsEmu;
	}

	ISteamUtils *GetISteamUtils( HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamUtils");
		return (ISteamUtils*)g_pSteamUtilsEmu;
	}

	ISteamBilling *GetISteamBilling( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamBilling");
		return NULL;
	}

	ISteamMatchmaking *GetISteamMatchmaking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamMatchmaking");
		return (ISteamMatchmaking*)g_pSteamMatchMakingEmu;
	}

	ISteamContentServer *GetISteamContentServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamContentServer");
		return NULL;
	}

	ISteamApps *GetISteamApps( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamApps");
		return NULL;
	}

	ISteamUserStats *GetISteamUserStats( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamUserStats");
		return (ISteamUserStats*)g_pSteamUStatsEmu;
	}

	ISteamNetworking *GetISteamNetworking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamNetworking");
		return (ISteamNetworking*)g_pSteamNetworkingEmu;
	}

	ISteamRemoteStorage *GetISteamRemoteStorage( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		return (ISteamRemoteStorage*)g_pSteamRemoteStorageEmu;
	}

	ISteamMasterServerUpdater *GetISteamMasterServerUpdater( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamMasterServerUpdater");
		return (ISteamMasterServerUpdater*)g_pSteamMasterServerUpdaterEmu;
	}

	ISteamMatchmakingServers *GetISteamMatchmakingServers( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GetISteamMatchmakingServers");
		return NULL;
	}

	void *GetISteamGenericInterface( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) {
		g_Logging.AddToLogFileA("ISteamClient.log", "GISGI");
		return NULL;
	}

	void RunFrame() {
		return;
	}

	uint32 GetIPCCallCount() {
		return 0;
	}

	void SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction ) {

	}
};

CSteamClient009*			g_pSteamClientEmu			= new CSteamClient009;

// for strings
unsigned int oneAtATimeHash(char* inpStr)
{
	unsigned int value = 0,temp = 0;
	for(size_t i=0;i<strlen(inpStr);i++)
	{
		char ctext = tolower(inpStr[i]);
		temp = ctext;
		temp += value;
		value = temp << 10;
		temp += value;
		value = temp >> 6;
		value ^= temp;
	}
	temp = value << 3;
	temp += value;
	unsigned int temp2 = temp >> 11;
	temp = temp2 ^ temp;
	temp2 = temp << 15;
	value = temp2 + temp;
	if(value < 2) value += 2;
	return value;
}

typedef hostent* (WINAPI *gethostbyname_t)(const char* name);
gethostbyname_t realgethostbyname = (gethostbyname_t)gethostbyname;

char* serverName = NULL;
char* webName = NULL;

__int64 _publicHostKey;

hostent* WINAPI custom_gethostbyname(const char* name) {
	// if the name is IWNet's stuff...
	unsigned int ip1 = oneAtATimeHash("ip1.pc.iw4.iwnet.infinityward.com");
	unsigned int log1 = oneAtATimeHash("log1.pc.iw4.iwnet.infinityward.com");
	unsigned int match1 = oneAtATimeHash("match1.pc.iw4.iwnet.infinityward.com");
	unsigned int web1 = oneAtATimeHash("web1.pc.iw4.iwnet.infinityward.com");
	unsigned int blob1 = oneAtATimeHash("blob1.pc.iw4.iwnet.infinityward.com");

	unsigned int current = oneAtATimeHash((char*)name);
	char* hostname = (char*)name;

	if (current == log1 || current == match1 || current == blob1 || current == ip1) {
		if (!serverName) {
			// test connectivity to hardcoded IP
			IPAddress^ address = gcnew IPAddress(0x3013175E); // 94.23.19.48, endian-flipped
			IPEndPoint^ ep = gcnew IPEndPoint(address, 13000);

			String^ es = ep->ToString();

			Sockets::Socket^ client = gcnew Sockets::Socket(Sockets::AddressFamily::InterNetwork, Sockets::SocketType::Stream, Sockets::ProtocolType::Tcp);
			client->Blocking = false;

			bool hostUp = true;//false;//true;

			//Windows::Forms::MessageBox::Show("NOTE: THIS IS A DEBUG BUILD (HOSTUP FORCING). CHECK STEAM_API.CPP TO DISABLE.");

			try {
				client->Connect(ep);
			} catch (Sockets::SocketException^ ex) {
				if (ex->NativeErrorCode != 10035) {
					hostUp = false;
				} else {
					if (!client->Poll(500 * 1000, Sockets::SelectMode::SelectWrite)) {
						hostUp = false;
					}
				}
			}

			if (!hostUp)
			{
				CIniReader reader(".\\IranRev.ini");
				serverName = reader.ReadString("Configuration", "Server", "server.iw4.ir"/*"dlc1-log1.pc.iw4.iwnet.infinityward.com"*/ );
			}
			else
			{
				in_addr addr;
				addr.S_un.S_addr = 0x3013175E;
				/*cli::array<Byte>^ keyData = gcnew cli::array<Byte>(8);
				client->Receive(keyData, 8, SocketFlags::None);

				_publicHostKey = BitConverter::ToInt64(keyData, 0);*/

				_publicHostKey = 0;

				serverName = inet_ntoa(addr);
			}

			client->Close();
		}

		hostname = serverName;
	}

	/*if (current == ip1) {
	if (!serverName) {
	CIniReader reader(".\\alterIWnet.ini");
	serverName = reader.ReadString("Configuration", "Server", "dlc1-ip1.pc.iw4.iwnet.infinityward.com");
	}

	hostname = serverName;
	}*/

	if (current == web1) {
		if (!webName) {
			CIniReader reader(".\\IranRev.ini");
			webName = reader.ReadString("Configuration", "WebServer", "server.iw4.ir"/*"web1.pc.iw4.iwnet.infinityward.com"*/);
		}

		hostname = webName;
	}

	return realgethostbyname(hostname);
}

#pragma unmanaged
VOID UpdateValidation(BOOL isInitial);
VOID UpdateRemoteFAL();

// why is this function so messy? okay, let's say *this file* is messy.
VOID FALRunner() {
	srand(time(NULL));

	//char time[64];
	// stack data to cause the crash to be proper

	while (true) {
		int sleepTime = (rand() % 5000) + 5000;
		//int sleepTime = 1;

		//sprintf(time, "SLEEP: %d", sleepTime);
		//OutputDebugStringA(time);

		Sleep(sleepTime);

		//OutputDebugStringA("VALIDATION");
		//UpdateValidation(TRUE);
		//UpdateRemoteFAL();

		//serverName = 0;

		//return;
	}
}
#pragma managed

bool VerifyMW2();
//void ShowLogo();


void FALThread() {
	//FALRunner();
}

void CheckThread() {
	/*while (true) {
	Sleep(50);

	Decompose();
	ResetWindowTitle();
	}*/
}
/*
void RunFrameHook() {
Custom::Hook->G_RunFrame();
}
void SayHook(const char* name, const char* text) {
Custom::Hook->G_Say(gcnew String(name), gcnew String(text));
}

void SayHook2(const char* name, const char* text) {
Custom::Hook->G_Say2(gcnew String(name), gcnew String(text));
}

}*/

void RunFrameHook() {
	if (Custom::Hook == nullptr) return;
	Custom::Hook->G_RunFrame();
}
void SayHook(const char* name, const char* text) {
	if (Custom::Hook == nullptr) return;
	Custom::Hook->G_Say(gcnew String(name), gcnew String(&text[1]));
	//Custom::Hook->G_Say(gcnew String(name), gcnew String(text));
}
void SayHook2(const char* name, const char* text) {
	if (Custom::Hook == nullptr) return;
	Custom::Hook->G_Say2(gcnew String(name), gcnew String(&text[1]));
	//Custom::Hook->G_Say(gcnew String(name), gcnew String(text));
}

extern "C"
{
	__declspec(dllexport) HSteamPipe __cdecl GetHSteamPipe()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "GetHSteamPipe" );

		return NULL;
	}
	__declspec(dllexport) HSteamUser __cdecl GetHSteamUser()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "GetHSteamUser" );

		return NULL;
	}
	__declspec(dllexport) HSteamPipe __cdecl SteamAPI_GetHSteamPipe()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_GetHSteamPipe" );

		return NULL;
	}
	__declspec(dllexport) HSteamUser __cdecl SteamAPI_GetHSteamUser()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_GetHSteamUser" );

		return NULL;
	}
	__declspec(dllexport) const char *__cdecl SteamAPI_GetSteamInstallPath()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_GetSteamInstallPath" );

		return NULL;
	}

	void HandleException(Object^ sender, UnhandledExceptionEventArgs^ e) {
		Windows::Forms::MessageBox::Show(e->ExceptionObject->ToString());
	}

	__declspec(dllexport) bool __cdecl SteamAPI_Init()
	{
		//ShowLogo();
		/*
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		if (System::Diagnostics::Process::GetProcessesByName("aIW")->Length > 0) {
		goto skipAiw;
		}

		// Start the child process. 
		if( !CreateProcessW( L"aIW.dll",   // No module name (use command line)
		L"",
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
		) 
		{


		}

		while (true) {
		Sleep(1);

		if (File::Exists("zone\\updateDone.dat")) {
		File::Delete("zone\\updateDone.dat");
		break;
		}

		if (GetProcessId(pi.hProcess) == NULL) {
		break;
		}
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		skipAiw:
		*/
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_Init" );

		//		Decompose();

		/*try {
		StatBackend::GetInt("TEST_STAT");
		StatBackend::Set("TEST_STAT", 1);
		StatBackend::Save();
		} catch (Exception^ ex) {
		System::Windows::Forms::MessageBox::Show(ex->ToString());
		}*/

		AppDomain::CurrentDomain->UnhandledException += gcnew UnhandledExceptionEventHandler(&HandleException);

		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_Init/b" );

		PBYTE offset = (PBYTE)GetProcAddress(GetModuleHandleA("ws2_32.dll"),"gethostbyname");
		realgethostbyname = (gethostbyname_t)DetourFunction(offset, (PBYTE)&custom_gethostbyname);

		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_Init/c" );

		Assembly^ assembly = Assembly::LoadFrom("aIW.dll");
		cli::array<Type^>^ types = assembly->GetTypes();

		for each (Type^ type in types) {
			if (type->IsSubclassOf(SteamAPI::SteamAPIHookBase::typeid)) {
				//Custom::Hook = (SteamAPI::SteamAPIHookBase^)Activator::CreateInstance(type);
				break;
			}
		}

		if (Custom::Hook != nullptr) {
			Custom::Hook->PerformUpdate();
		}

		useNewAuthFunctions = false;

		// needs to be here for 'unban' checking
		GetPlayerSteamID();

		//LoadLibraryA("cgui.dll");

		bool result = VerifyMW2();

		if (result && Custom::Hook != nullptr) {
			CIniReader reader(".\\IranRev.ini");
			serverName = reader.ReadString("Configuration", "Server", "server.iw4.ir"/*"log1.pc.iw4.iwnet.infinityward.com"*/);

			//useNewAuthFunctions = !reader.ReadBoolean("Configuration", "DisableAuth", true);
			//useNewAuthFunctions = (!strcmp(serverName, "server.alteriw.net"));
			useNewAuthFunctions = true;

			if (useNewAuthFunctions || true) {
				result = Custom::Hook->SteamAPIInit(gcnew String(serverName));
			}
		}

		if (!result) {
			Diagnostics::Process::GetCurrentProcess()->Kill();
		}

		Thread^ thread = gcnew Thread(gcnew ThreadStart(CheckThread));
		thread->Start();

		Thread^ thread2 = gcnew Thread(gcnew ThreadStart(FALThread));
		thread2->Start();

		//InstallValidator();

		return true;
	}

	__declspec(dllexport) bool __cdecl SteamAPI_InitSafe()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_InitSafe" );

		return true;
	}

	__declspec(dllexport) char __cdecl SteamAPI_RestartApp()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_RestartApp" );

		return 1;
	}

	__declspec(dllexport) char __cdecl SteamAPI_RestartAppIfNecessary()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_RestartAppIfNecessary" );

		return 0;
	}

	__declspec(dllexport) void __cdecl SteamAPI_RegisterCallResult( CCallbackBase* pResult, SteamAPICall_t APICall )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_RegisterCallResult for call %d", APICall );

		Callbacks::RegisterResult(pResult, APICall);
		//
	}

	__declspec(dllexport) void __cdecl SteamAPI_RegisterCallback( CCallbackBase *pCallback, int iCallback )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_RegisterCallback for call type %d", iCallback );

		Callbacks::Register(pCallback, iCallback);
		//
	}
	__declspec(dllexport) void __cdecl SteamAPI_RunCallbacks()
	{
		//Logger( "steam_emu", "SteamAPI_RunCallbacks" );

		Callbacks::Run();
		//
	}
	__declspec(dllexport) void __cdecl SteamAPI_SetMiniDumpComment( const char *pchMsg )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_SetMiniDumpComment" );

		//
	}
	__declspec(dllexport) void __cdecl SteamAPI_SetTryCatchCallbacks( bool bUnknown )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_SetTryCatchCallbacks" );

		// return bUnknown;
	}
	__declspec(dllexport) void __cdecl SteamAPI_Shutdown()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_Shutdown" );

		//
	}
	__declspec(dllexport) void __cdecl SteamAPI_UnregisterCallResult( CCallbackBase* pResult, SteamAPICall_t APICall )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_UnregisterCallResult" );

		//
	}
	__declspec(dllexport) void __cdecl SteamAPI_UnregisterCallback( CCallbackBase *pCallback, int iCallback )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_UnregisterCallback" );

		Callbacks::Unregister(pCallback);
		//
	}
	__declspec(dllexport) void __cdecl SteamAPI_WriteMiniDump( uint32 uStructuredExceptionCode, void* pvExceptionInfo, uint32 uBuildID )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_WriteMiniDump" );

		//
	}
	__declspec(dllexport) ISteamApps003* __cdecl SteamApps()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamApps" );

		return (ISteamApps003*)g_pSteamAppsEmu;
	}
	__declspec(dllexport) ISteamClient009* __cdecl SteamClient()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamClient" );

		return (ISteamClient009*)g_pSteamClientEmu;
	}
	__declspec(dllexport) ISteamContentServer002* __cdecl SteamContentServer()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamContentServer" );

		return NULL;
	}
	__declspec(dllexport) ISteamUtils005* __cdecl SteamContentServerUtils()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamContentServerUtils" );

		return NULL;
	}
	__declspec(dllexport) bool __cdecl SteamContentServer_Init( unsigned int uLocalIP, unsigned short usPort )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamContentServer_Init" );

		return NULL;
	}
	__declspec(dllexport) void __cdecl SteamContentServer_RunCallbacks()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamContentServer_RunCallbacks" );

		//
	}
	__declspec(dllexport) void __cdecl SteamContentServer_Shutdown()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamContentServer_Shutdown" );

		//
	}
	__declspec(dllexport) ISteamFriends005* __cdecl SteamFriends()
	{
		//Logger( "steam_emu", "SteamFriends" );

		return (ISteamFriends005*)g_pSteamFriendsEmu;
	}
	__declspec(dllexport) ISteamGameServer010* __cdecl SteamGameServer()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamGameServer" );

		return (ISteamGameServer010*)g_pSteamGameServerEmu;
	}
	__declspec(dllexport) ISteamUtils005* __cdecl SteamGameServerUtils()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamGameServerUtils" );

		return NULL;
	}
	__declspec(dllexport) bool __cdecl SteamGameServer_BSecure()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamGameServer_BSecure" );

		return true;
	}
	__declspec(dllexport) HSteamPipe __cdecl SteamGameServer_GetHSteamPipe()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamGameServer_GetHSteamPipe" );

		return NULL;
	}
	__declspec(dllexport) HSteamUser __cdecl SteamGameServer_GetHSteamUser()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamGameServer_GetHSteamUser" );

		return NULL;
	}
	__declspec(dllexport) int32 __cdecl SteamGameServer_GetIPCCallCount()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamGameServer_GetIPCCallCount" );

		return NULL;
	}
	__declspec(dllexport) uint64 __cdecl SteamGameServer_GetSteamID()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamGameServer_GetSteamID" );

		return NULL;
	}
	__declspec(dllexport) bool __cdecl SteamGameServer_Init( uint32 unIP, uint16 usPort, uint16 usGamePort, EServerMode eServerMode, int nGameAppId, const char *pchGameDir, const char *pchVersionString )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamGameServer_Init" );

		return true;
	}
	__declspec(dllexport) bool __cdecl SteamGameServer_InitSafe( uint32 unIP, uint16 usPort, uint16 usGamePort, EServerMode eServerMode, int nGameAppId, const char *pchGameDir, const char *pchVersionString, unsigned long dongs )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamGameServer_InitSafe" );

		return true;
	}
	__declspec(dllexport) void __cdecl SteamGameServer_RunCallbacks()
	{
		//Logger( "steam_emu", "SteamGameServer_RunCallbacks" );

		//
	}
	__declspec(dllexport) void __cdecl SteamGameServer_Shutdown()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamGameServer_Shutdown" );
		//
	}
	__declspec(dllexport) ISteamMasterServerUpdater001* __cdecl SteamMasterServerUpdater()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamMasterServerUpdater" );

		return (ISteamMasterServerUpdater001*)g_pSteamMasterServerUpdaterEmu;
	}
	__declspec(dllexport) ISteamMatchmaking008* __cdecl SteamMatchmaking()
	{
		//Logger( "steam_emu", "SteamMatchmaking" );
		return (ISteamMatchmaking008*)g_pSteamMatchMakingEmu;
	}
	__declspec(dllexport) ISteamMatchmakingServers002* __cdecl SteamMatchmakingServers()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamMatchmakingServers" );

		return NULL;
	}
	__declspec(dllexport) ISteamNetworking003* __cdecl SteamNetworking()
	{
		//Logger( "steam_emu", "SteamNetworking" );

		return (ISteamNetworking003*)g_pSteamNetworkingEmu;
	}
	//__declspec(dllexport) void* __cdecl SteamRemoteStorage()
	__declspec(dllexport) ISteamRemoteStorage002* __cdecl SteamRemoteStorage()
	{
		//Logger( "steam_emu", "SteamRemoteStorage" );

		return g_pSteamRemoteStorageEmu;
	}
	__declspec(dllexport) ISteamUser013* __cdecl SteamUser()
	{
		//Logger( "steam_emu", "SteamUser" );
		g_Logging.AddToLogFileA( "steam_emu.log", "Steam_RunCallbacks" );

		return (ISteamUser013*)g_pSteamUserEmu;
	}
	__declspec(dllexport) ISteamUserStats007* __cdecl SteamUserStats()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamUserStats" );

		return (ISteamUserStats007*)g_pSteamUStatsEmu;
	}
	__declspec(dllexport) ISteamUtils005* __cdecl SteamUtils()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "SteamUtils" );

		return (ISteamUtils005*)g_pSteamUtilsEmu;
	}
	__declspec(dllexport) HSteamUser __cdecl Steam_GetHSteamUserCurrent()
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "Steam_GetHSteamUserCurrent" );

		return NULL;
	}
	__declspec(dllexport) void __cdecl Steam_RegisterInterfaceFuncs( void *hModule )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "Steam_RegisterInterfaceFuncs" );

		//
	}
	__declspec(dllexport) void __cdecl Steam_RunCallbacks( HSteamPipe hSteamPipe, bool bGameServerCallbacks )
	{
		g_Logging.AddToLogFileA( "steam_emu.log", "Steam_RunCallbacks" );

		//
	}
	__declspec(dllexport) void *g_pSteamClientGameServer = NULL;
}

#pragma unmanaged

void PatchMW2();

BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	if( ul_reason_for_call == DLL_PROCESS_ATTACH )
	{
		PatchMW2();
		UpdateValidation(TRUE);

		g_Logging.BaseUponModule( hModule );
		g_Logging.AddToLogFileA( "steam_emu.log", "Attached to process!" );
	}

	return TRUE;
}