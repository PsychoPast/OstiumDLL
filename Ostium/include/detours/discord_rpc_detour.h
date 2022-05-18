#pragma once

#include "pch.h"
#include "../libs/discord-rpc/include/discord_rpc.h"

decltype(Discord_Initialize)*discordInitialize_;
decltype(Discord_UpdatePresence)* discordUpdatePresence_;

void Discord_Initialize_Detour(const char* applicationId, DiscordEventHandlers* handlers, int autoRegister, const char* optionalSteamId)
{
	applicationId = "869289675393732658";
	discordInitialize_(applicationId, handlers, autoRegister, optionalSteamId);
}

void Discord_UpdatePresence_Detour(const DiscordRichPresence* presence)
{
	const DiscordRichPresence newPresence
	{
		presence->state,
		presence->details,
		presence->startTimestamp,
		presence->endTimestamp,
		"ost_logo",
		presence->largeImageText,
		presence->smallImageKey,
		presence->smallImageText,
		presence->partyId,
		presence->partySize,
		presence->partyMax,
		presence->matchSecret,
		presence->joinSecret,
		presence->spectateSecret,
		presence->instance
	};

	discordUpdatePresence_(&newPresence);
}