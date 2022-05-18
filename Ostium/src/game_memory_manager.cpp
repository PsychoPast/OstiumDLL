#include "pch.h"
#include "game_memory_manager.h"
#include "patterns.h"
#include "util.h"
#include "../libs/discord-rpc/include/discord_rpc.h"
#include "detours/get_proc_address_detour.h"

GameMemoryManager::GameMemoryManager()
{
	printf_s("Initialzing...\n");
	
	// by passing nullptr to the function, we're retrieving the game module
	if(!getAddressAndSizeOfModule(nullptr, mainModuleBaseAddress, mainModuleSize))
	{
		printf_s("Couldn't get the address and size of the game module. Error code: %lu", GetLastError());
		
		//funny number
		exit(69);
	}
	
	printf_s("Module address: 0x%p\nModule size: %lu\n", mainModuleBaseAddress, mainModuleSize);

	sSlManager = {};
}

void GameMemoryManager::Init()
{
	// find GEngine
	{
		const auto gEnginePattern = findPattern(mainModuleBaseAddress, mainModuleSize, Patterns::Gengine.pattern, Patterns::Gengine.mask, Patterns::Gengine.size);
		if (!gEnginePattern)
		{
			printf_s("Couldn't find GEngine address, exiting...");
			exit(69);
		}

		// in assembly x64, a global variable (in that case, GEngine of type UEngine*) address is relative to the instruction pointer register (called RIP). The RIP holds a pointer to the next instruction.
		// in our case, 'gEnginePattern' gives us the address of 'mov GEngine, rax'. 'gEnginePattern + 7' points to the next instruction (test rax,rax) which is basically the RIP value.gEnginePattern + 3 points to the relative address (4 bytes int) that gets added to the RIP value to obtain the address of GEngine
		// the expression between () gives us the address of GEngine(UEngine*). Therefore we cast it to uinpt_ptr then dereference it to obtain its value (same as casting to UEngine** and dereferencing it)
		const auto pGengine = *reinterpret_cast<uintptr_t*>(gEnginePattern + 7 + *reinterpret_cast<uint32_t*>(gEnginePattern + 3));
		gEngine = reinterpret_cast<UEngine*>(pGengine);
#if _DEBUG
		printf_s("GEngine address: %p\n", pGengine);
#endif

	}
	
	// get StaticConstructObject_Internal
	{
		const auto p_soci = findPattern(mainModuleBaseAddress, mainModuleSize, Patterns::Soci.pattern, Patterns::Soci.mask, Patterns::Soci.size);
		if (!p_soci)
		{
			printf_s("Couldn't find StaticConstructObject_Internal address, exiting...");
			exit(69);
		}
		
		sOci = reinterpret_cast<StaticObjectConstructInt>(p_soci);
#if _DEBUG
		printf_s("SOC_I address: %p\n", p_soci);
#endif
	}
}

template<typename UType>
UType* GameMemoryManager::StaticObjectConstruct_Internal(const FStaticConstructObjectParameters params)
{
	return reinterpret_cast<UType*>(sOci(params));
}

UConsole* GameMemoryManager::GetOrCreateConsole()
{
	if(const auto inConsole = gEngine->gameViewport->viewPortConsole)
	{
		return inConsole;
	}

#if _DEBUG
	printf_s("The console is stripped off the game, constructing a new object...\n");
#endif
	
	const FStaticConstructObjectParameters params
	{
		gEngine->consoleClass,
		 gEngine->gameViewport,
		  FName{{0},0},
	   0,
 0,
		false,
		false,
		nullptr,
		nullptr,
		nullptr,
	};
		
	return StaticObjectConstruct_Internal<UConsole>(params);
}

void GameMemoryManager::HookDiscordRpc()
{
	const std::pair<const char*, std::function<FARPROC(HMODULE, LPCSTR)>> dI
	{
		"Discord_Initialize",
		[](HMODULE hModule, LPCSTR lpProcName)
		{
			discordInitialize_ = reinterpret_cast<decltype(Discord_Initialize)*>(getProcAddress_(hModule, lpProcName));
			return reinterpret_cast<FARPROC>(&Discord_Initialize_Detour);
		}
	};
			
	const std::pair<const char*, std::function<FARPROC(HMODULE, LPCSTR)>> dUE
	{
		"Discord_UpdateEvent",
		[](HMODULE hModule, LPCSTR lpProcName)
		{
			// since this func gets called multiple times, we only need to set the original function ONCE
		if (!discordUpdatePresence_)
		{
			discordUpdatePresence_ = reinterpret_cast<decltype(Discord_UpdatePresence)*>(getProcAddress_(hModule, lpProcName));
		}

		return reinterpret_cast<FARPROC>(&Discord_UpdatePresence_Detour);
		}
	};

	activeHooks_.push_back(dI);
	activeHooks_.push_back(dUE);
}

void GameMemoryManager::HookEqu8()
{
	const std::pair<const char*, std::function<FARPROC(HMODULE, LPCSTR)>> init
	{
		"equ8_client_initialize",
		[](HMODULE, LPCSTR)
		{
			return reinterpret_cast<FARPROC>(&Equ8ClientInitializeDetour);
		}
	};

	const std::pair<const char*, std::function<FARPROC(HMODULE, LPCSTR)>> pollEvent
	{
		"equ8_client_poll_event",
		[](HMODULE, LPCSTR)
		{
		    return reinterpret_cast<FARPROC>(&Equ8ClientPollEventDetour);
		}
	};

	activeHooks_.push_back(init);
	activeHooks_.push_back(pollEvent);
}