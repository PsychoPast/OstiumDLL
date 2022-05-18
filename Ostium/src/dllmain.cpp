#include "pch.h"
#include "game_memory_manager.h"
#include "util.h"
#include "detours/get_proc_address_detour.h"
#include "hooks/iat_hook.h"

bool HookGetProcAddr()
{
    // we use IAT hook to hook to kernel32::GetProcAddress
    auto hook = IATHook("kernel32.dll", "GetProcAddress", reinterpret_cast<uintptr_t>(&getProcAddressDetour));

    // we set the function pointer to the "original function" which in this case is the authentic GetProcAddress
    getProcAddress_ = reinterpret_cast<decltype(GetProcAddress)*>(hook.GetOriginalFunc());
    return hook.Hook();
}

void entry()
{
    // alloc a new console to redirect output
    attachConsoleToProcess();

    // hook GetProcAddress
    if (!HookGetProcAddr())
    {
        printf_s("Could not hook GetProcAddress.");
    }

	GameMemoryManager mem{};

    // hook equ8 and discord-rpc
    mem.HookEqu8();
    mem.HookDiscordRpc();

    // patch ssl
    if(!mem.sSlManager.PatchHTTP() || !mem.sSlManager.PatchWSS())
    {
        exit(69);
    }

    // init (patterns and stuff)
    mem.Init();

    // get a UConsole instance
    UConsole* console = mem.GetOrCreateConsole();
    if (!console)
    {
        printf_s("Couldn't get or construct a UConsole instance.");
        exit(69);
    }
    
#if _DEBUG
    printf_s("UConsole instance: (%p)\n", console);
#endif

    // we travel to the lobby
    console->ConsoleCommand(FString{ L"travel lobby?game=lobby" });
}
    
BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        entry();
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}