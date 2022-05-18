#pragma once

#include "pch.h"
#include <functional>
#include <vector>
#include "discord_rpc_detour.h"
#include "equ8_detour.h"

inline decltype(GetProcAddress)* __stdcall getProcAddress_;
inline std::vector<std::pair<const char*, std::function<FARPROC(HMODULE, LPCSTR)>>> activeHooks_;

bool isHooked(LPCSTR lpProcName, std::function<FARPROC(HMODULE, LPCSTR)>& callback)
{
	const auto func = std::find_if(activeHooks_.begin(), activeHooks_.end(), [lpProcName](const std::pair<const char*, std::function<FARPROC(HMODULE, LPCSTR)>>& funcPair) { return funcPair.first == lpProcName; });

	// it means that the function is not hooked
	if(func == activeHooks_.end())
	{
		return false;
	}

	callback = func->second;
	return true;
}

FARPROC __stdcall getProcAddressDetour(HMODULE hModule, LPCSTR lpProcName)
{
	// according to microsoft, 'lpProcName' can either be a char* or a number. to make sure it isn't an ordinal value we check if the high-order word is 0
	if (!HIWORD(lpProcName))
	{
		// we cannot compare a number with a string therefore we simply call the original function
		return getProcAddress_(hModule, lpProcName);
	}

	std::function<FARPROC(HMODULE, LPCSTR)> callback;
	if(isHooked(lpProcName, callback))
	{
#if _DEBUG
		printf_s("GetProcAddress: Intercepted '%s'\n", lpProcName);
#endif

		return callback(hModule, lpProcName);
	}

	return getProcAddress_(hModule, lpProcName);
}