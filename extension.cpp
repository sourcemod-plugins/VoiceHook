#include "extension.h"

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

VoiceHook g_VoiceHook;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_VoiceHook);

SH_DECL_HOOK1_void(IServerGameClients, ClientVoice, SH_NOATTRIB, 0, edict_t *);

IServerGameClients *gameclients = NULL;
IForward *g_OnClientSpeaking = NULL;

void VoiceHook::ClientVoice( edict_t *pEntity )
{
	if (pEntity)
	{
		int client = gamehelpers->IndexOfEdict(pEntity);
		if (client)
		{
			g_OnClientSpeaking->PushCell(client);
			g_OnClientSpeaking->Execute();
		}
	}
}

bool VoiceHook::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientVoice, gameclients, this, &VoiceHook::ClientVoice, true);
	return true;
}

bool VoiceHook::SDK_OnMetamodUnload(char *error, size_t maxlength)
{
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientVoice, gameclients, this, &VoiceHook::ClientVoice, true);
	return true;
}

bool VoiceHook::SDK_OnLoad(char *error, size_t maxlength, bool late)
{	
	return true;
}

void VoiceHook::SDK_OnAllLoaded()
{
	g_OnClientSpeaking = forwards->CreateForward("OnClientSpeaking", ET_Event, 1, NULL, Param_Cell);
}

void VoiceHook::SDK_OnUnload()
{
	forwards->ReleaseForward(g_OnClientSpeaking);
}
