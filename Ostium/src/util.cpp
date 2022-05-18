#include "pch.h"
#include "util.h"
#include <Psapi.h>
#include <string>

void attachConsoleToProcess()
{
	// we allocate a console to the process
	// there's no need to check if the function was successful since we can't log the error yet at that point anw
	AllocConsole();

	// we redirect both, the game log and ours to the allocated console
	FILE* notNeeded;
	freopen_s(&notNeeded, "CONOUT$", "w", stdout);
}

bool getAddressAndSizeOfModule(const char* moduleName, uintptr_t& moduleAddress, DWORD& moduleSize)
{
	const HANDLE proc = GetCurrentProcess();
	const HMODULE h_main_module = GetModuleHandleA(moduleName);
	if (!h_main_module)
	{
		return false;
	}

	// we get the module name
	char moduleNameBuffer[MAX_PATH];
	DWORD nameLength = GetModuleBaseNameA(proc, h_main_module, moduleNameBuffer, sizeof(moduleNameBuffer));
	if (nameLength)
	{
		const auto moduleName = std::string(moduleNameBuffer, nameLength);
		printf_s("Module name: %s\n", moduleName.c_str());
	}

	// we get the infos of the module
	MODULEINFO module_info = {};
	if (!GetModuleInformation(proc, h_main_module, &module_info, sizeof(module_info)))
	{
		return false;
	}

	moduleAddress = reinterpret_cast<uintptr_t>(module_info.lpBaseOfDll);
	moduleSize = module_info.SizeOfImage;
	return true;
}

bool iterateThroughPattern(PBYTE currentAddress, const char* pattern, const char* mask, DWORD size)
{
	// address of the pattern end
	const auto end = currentAddress + size;
	for (auto start = currentAddress; start < end; pattern++, mask++, start++)
	{
		// we compare the char at the current against against the char in the pattern
		if (*start != static_cast<BYTE>(*pattern) && *mask != '?')
		{
			return false;
		}
	}

	return true;
}

PBYTE findPattern(uintptr_t baseAddress, DWORD size, const char* pattern, const char* mask, DWORD patternSize)
{
	// last address of the module
	const auto end = reinterpret_cast<PBYTE>(static_cast<uintptr_t>(size) + baseAddress);
	for (auto adr = reinterpret_cast<PBYTE>(baseAddress); adr < end; adr++)
	{
		// we're passing a COPY of the pointer, therefore it won't be modified in "iterate_through_pattern"
		if (iterateThroughPattern(adr, pattern, mask, patternSize))
		{
			return adr;
		}
	}

	return nullptr;
}