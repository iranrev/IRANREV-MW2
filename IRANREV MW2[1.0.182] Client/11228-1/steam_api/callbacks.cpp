#include "stdafx.h"
#include "callbacks.h"
#include "AppTools.h"

#if fuck
CallbackManager callbackmanager;

void CallbackProvider::ResolveExports()
{
	getcallback = (SteamBGetCallbackFn)GetProcAddress(module, "Steam_BGetCallback");
	freelastcallback = (SteamFreeLastCallbackFn)GetProcAddress(module, "Steam_FreeLastCallback");
	getapicallresult = (SteamGetAPICallResultFn)GetProcAddress(module, "Steam_GetAPICallResult");
}

void CallbackProvider::Set(HMODULE mod)
{
	if(module == mod)
		return;

	module = mod;
	ResolveExports();
}

void CallbackManager::Cleanup()
{
	callbacks.clear();
	apicalls.clear();
}

void CallbackManager::SetCallbackProvider(HMODULE module)
{
	provider.Set(module);
}

void CallbackManager::RunAPICallbacks(HSteamPipe pipe, SteamAPICallCompleted_t *call)
{
	SteamAPICall_t apicall = call->m_hAsyncCall;

	APICallsMap::const_iterator iter = apicalls.find(apicall);

	if(iter == apicalls.end())
		return;

	CCallbackBase *callback = iter->second;

	int size = callback->GetCallbackSizeBytes();
	void *data = malloc(size);

	bool failed = false;
	bool iok = provider.Steam_GetAPICallResult(pipe, apicall, data, size, callback->GetICallback(), &failed);

	callback->Run(data, (!iok || failed), apicall);

	free(data);
}

void CallbackManager::RunCallbacks()
{
	CallbackMsg_t callbackMsg;
	HSteamCall steamCall;

	//g_Logging.AddToLogFileA("steam_emu.log", "RunCallbacks_i. callbackMsg: %d, steamCall: %d", callbackMsg, steamCall);

	while( provider.Steam_BGetCallback(NULL, &callbackMsg, &steamCall) )
	{
		currentUser = callbackMsg.m_hSteamUser;

		int32 callBack = callbackMsg.m_iCallback;
		ECallbackType type = (ECallbackType)((callBack / 100) * 100);
		//std::cout << "[DEBUG] Callback: " << callBack << ", Type: " << EnumString<ECallbackType>::From(type) << ", Size: " << callbackMsg.m_cubParam << std::endl;
		g_Logging.AddToLogFileA( "steam_emu.log", "Callback %d Type %s Size %d", callBack, EnumString<ECallbackType>::From(type).c_str(), callbackMsg.m_cubParam );

		if(callbackMsg.m_iCallback == SteamAPICallCompleted_t::k_iCallback)
		{
			SteamAPICallCompleted_t *call = (SteamAPICallCompleted_t *)callbackMsg.m_pubParam;
			RunAPICallbacks(NULL, call);
		}

		std::pair<CallbacksMap::iterator, CallbacksMap::iterator> range = callbacks.equal_range(callbackMsg.m_iCallback);
		for(CallbacksMap::const_iterator iter = range.first; iter != range.second; ++iter)
		{
			CCallbackBase *callback = iter->second;

			if(false && !(callback->m_nCallbackFlags & CCallbackBase::k_ECallbackFlagsGameServer))
				continue;

			if(callbackTryCatch)
			{

				try {
					callback->Run(callbackMsg.m_pubParam);
				} catch(...) {};

			} else {
				callback->Run(callbackMsg.m_pubParam);
			}
		}

		provider.Steam_FreeLastCallback(NULL);
	}
}




void CallbackManager::RegisterCallback(int iCallback, CCallbackBase *callback)
{
	// since the property is there I'm going to set it so we have it for unregister, I'm not sure if the real steam_api uses it
	// it has to be safe because if you were to register a CCallback with multiple callback numbers, who knows what one would be erased
	callback->m_iCallback = iCallback;

	callbacks.insert(CallbacksMap::value_type(iCallback, callback));
}

void CallbackManager::UnRegisterCallback(CCallbackBase *callback)
{
	std::pair<CallbacksMap::iterator, CallbacksMap::iterator> range = callbacks.equal_range( callback->GetICallback() );

	CallbacksMap::const_iterator iter = range.first;
	while(iter != range.second)
	{
		if(iter->second == callback)
			iter = callbacks.erase(iter);
		else
			++iter;
	}
}




void CallbackManager::RegisterAPICallResult(SteamAPICall_t apiCall, CCallbackBase *callback)
{
	apicalls.insert(APICallsMap::value_type(apiCall, callback));
}

void CallbackManager::UnRegisterAPICallResult(SteamAPICall_t apiCall, CCallbackBase *callback)
{
	std::pair<APICallsMap::iterator, APICallsMap::iterator> range = apicalls.equal_range( apiCall );

	APICallsMap::const_iterator iter = range.first;
	while(iter != range.second)
	{
		if(iter->second == callback)
			iter = apicalls.erase(iter);
		else
			++iter;
	}
}




S_API void STEAM_CALL SteamAPI_RegisterCallback( class CCallbackBase *pCallback, int iCallback )
{
	g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_RegisterCallback" );
	callbackmanager.RegisterCallback(iCallback, pCallback);
}

S_API void STEAM_CALL SteamAPI_UnregisterCallback( class CCallbackBase *pCallback )
{
	g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_UnregisterCallback" );
	callbackmanager.UnRegisterCallback(pCallback);
}




S_API void STEAM_CALL SteamAPI_RegisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
	g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_RegisterCallResult" );
	callbackmanager.RegisterAPICallResult(hAPICall, pCallback);
}

S_API void STEAM_CALL SteamAPI_UnregisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
	g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_UnregisterCallResult" );
	callbackmanager.UnRegisterAPICallResult(hAPICall, pCallback);
}




S_API void STEAM_CALL SteamAPI_RunCallbacks()
{
	g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_RunCallbacks" );
	callbackmanager.RunCallbacks();
}

S_API void STEAM_CALL Steam_RegisterInterfaceFuncs(void *hModule)
{
	g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_RegisterInterfaceFuncs" );
	callbackmanager.SetCallbackProvider((HMODULE)hModule);
}



S_API HSteamUser STEAM_CALL Steam_GetHSteamUserCurrent()
{
	g_Logging.AddToLogFileA( "steam_emu.log", "Steam_GetHSteamUserCurrent" );
	return callbackmanager.currentUser;
}


S_API bool SteamAPI_SetTryCatchCallbacks( bool bTryCatchCallbacks )
{
	g_Logging.AddToLogFileA( "steam_emu.log", "SteamAPI_SetTryCatchCallbacks" );
	callbackmanager.callbackTryCatch = bTryCatchCallbacks;
	return bTryCatchCallbacks;
}
#endif