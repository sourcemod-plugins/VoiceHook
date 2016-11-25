#include "extension.h"

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

VoiceHook g_VoiceHook;		/**< Global singleton for extension's main interface */

SMEXT_LINK(&g_VoiceHook);

SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, edict_t *);
SH_DECL_HOOK2_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, edict_t *, char const *);

SH_DECL_MANUALHOOK0_void(OnVoiceTransmitHook, 0, 0, 0);

IServerGameClients *gameclients = NULL;

IGameConfig *pConfigFile = NULL;
IForward *g_OnClientSpeaking = NULL;
IServerGameEnts *gameents = NULL;

CGlobalVars *gpGlobals;

void VoiceHook::ClientPutInServer( edict_t *pEntity, char const *playername )
{
	CBaseEntity *pBase = gameents->EdictToBaseEntity(pEntity);
	if (pBase)
	{
		SH_ADD_MANUALHOOK_MEMFUNC(OnVoiceTransmitHook, pBase, &g_VoiceHook, &VoiceHook::CBaseEntityOnVoiceTransmit, true);
	}
}

void VoiceHook::ClientDisconnect( edict_t *pEntity )
{
	CBaseEntity *pBase = gameents->EdictToBaseEntity(pEntity);
	if (pBase)
	{
		SH_REMOVE_MANUALHOOK_MEMFUNC(OnVoiceTransmitHook, pBase, &g_VoiceHook, &VoiceHook::CBaseEntityOnVoiceTransmit, true);
	}
}

void VoiceHook::CBaseEntityOnVoiceTransmit()
{
	g_OnClientSpeaking->PushCell(IndexOfEdict(gameents->BaseEntityToEdict(META_IFACEPTR(CBaseEntity))));
	g_OnClientSpeaking->Execute();
}

bool VoiceHook::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, gameents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &VoiceHook::ClientDisconnect, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &VoiceHook::ClientPutInServer, true);
	gpGlobals = ismm->GetCGlobals();
	return true;
}

bool VoiceHook::SDK_OnMetamodUnload(char *error, size_t maxlength)
{
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &VoiceHook::ClientDisconnect, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &VoiceHook::ClientPutInServer, true);
	return true;
}

bool VoiceHook::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	char ConfigError[128];
	if(!gameconfs->LoadGameConfigFile("Voicehook", &pConfigFile, ConfigError, sizeof(ConfigError)))
	{
		if (error)
		{
			snprintf(error, maxlength, "Voicehook.txt error : %s", ConfigError);
		}
		return false;
	}
	int Offset = 0;
	pConfigFile->GetOffset("OnVoiceTransmit", &Offset);
	SH_MANUALHOOK_RECONFIGURE(OnVoiceTransmitHook, Offset, 0, 0);
	
	return true;
}

void VoiceHook::SDK_OnAllLoaded()
{
	g_OnClientSpeaking = forwards->CreateForward("OnClientSpeaking", ET_Event, 1, NULL, Param_Cell);
}

void VoiceHook::SDK_OnUnload()
{
	forwards->ReleaseForward(g_OnClientSpeaking);
	gameconfs->CloseGameConfigFile(pConfigFile);
}
