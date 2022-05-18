#pragma once

#include "ssl_manager.h"
#include "uetypes.h"

typedef UObject* (__fastcall* StaticObjectConstructInt)(FStaticConstructObjectParameters params);

class GameMemoryManager
{
public:
	
	SSLManager sSlManager;
	
	GameMemoryManager();
	void Init();
	UConsole* GetOrCreateConsole();
	void HookDiscordRpc();
	void HookEqu8();

private:
	
	StaticObjectConstructInt sOci;
	UEngine* gEngine;
	
	template<typename TUType>
	TUType* StaticObjectConstruct_Internal(FStaticConstructObjectParameters params);
};