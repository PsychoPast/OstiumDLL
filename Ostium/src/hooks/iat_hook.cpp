#include "pch.h"
#include "hooks/iat_hook.h"
#include <Psapi.h>
#include <cstdio>
#include <string>

PIMAGE_IMPORT_DESCRIPTOR IATHook::_importDirectoryTable;

IATHook::IATHook(const char* moduleName, const char* functionName, uintptr_t detour)
{

#if _DEBUG
	printf_s("Initializing IAT Hook for '%s::%s'...\n", moduleName, functionName);
#endif

	// if the importDirectoryTable is null, we set it
	if (!_importDirectoryTable)
	{
		_importDirectoryTable = GetImportDirectoryTable();

#if _DEBUG
		printf_s("Import Directory Table address: %p\n", _importDirectoryTable);
#endif
	}

	// we try to get the given function from the given module
	originalFunc = GetFunctionFromModule(moduleName, functionName);

	// usually, the import table page is read-only, therefore we must set the write permission
	if(originalFunc && EnablePageWriting(ptrToFunc))
	{
		canBeHooked = true;
	}

	detourFunc = detour;
}

PIMAGE_IMPORT_DESCRIPTOR IATHook::GetImportDirectoryTable()
{
	// we gt the MS-DOS Header
	const auto msDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(_imageBaseAddress);

	// we get the NT header
	const auto imageNTHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(FROM_RVA(msDosHeader->e_lfanew));

	// we get the import table
	return reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(FROM_RVA(imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress));
}

IMAGE_IMPORT_DESCRIPTOR* IATHook::GetModule(const char* moduleName)
{
	// we iterate throught the import directory table until we find the module that matches the one we're looking for
	// if Characteristics is 0, it means there are no more entries
	for (IMAGE_IMPORT_DESCRIPTOR* module = _importDirectoryTable; module->Characteristics; module++)
	{
		if(!_stricmp(moduleName, reinterpret_cast<char*>(FROM_RVA(module->Name))))
		{
#if _DEBUG
			printf_s("Found module '%s' in import table: %p\n", moduleName, module);
#endif
			return module;
		}
	}

	return nullptr;
}

bool IATHook::EnablePageWriting(void* address)
{
	MEMORY_BASIC_INFORMATION inf;

	if(VirtualQuery(address, &inf, sizeof(inf)))
	{
		if(!(inf.AllocationProtect & PAGE_READWRITE))
		{
			DWORD nothing;
			if(VirtualProtect(address, sizeof(uintptr_t), PAGE_READWRITE, &nothing))
			{
#if _DEBUG
				printf_s("Successfuly set read-write permissions for page containing address %p\n", address);
#endif

				return true;
			}
		}
		else
		{
#if _DEBUG
			printf_s("Page containing address %p already has read-write permissions\n", address);
#endif
			return true;
		}
	}

	printf_s("Could not set the page containing address %p 's permissions, error code: %lu\n", address, GetLastError());
	return false;
}

uintptr_t IATHook::GetFunctionFromModule(const char* moduleName, const char* functionName)
{
	if(const auto module = GetModule(moduleName))
	{
		return GetFunctionFromModule(*module, functionName);
	}

	printf_s("Could not find module '%s' in import table.\n", moduleName);
	return 0;
}

uintptr_t IATHook::GetFunctionFromModule(IMAGE_IMPORT_DESCRIPTOR module, const char* functionName)
{
	// OFT contains the function name, FT contains the function address
	if(!module.OriginalFirstThunk || !module.FirstThunk)
	{
		return 0;
	}

	auto firstThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(FROM_RVA(module.FirstThunk));
	auto oft = reinterpret_cast<PIMAGE_THUNK_DATA>(FROM_RVA(module.OriginalFirstThunk));

	for(; oft->u1.Function; firstThunk++, oft++)
	{
		// we skip ordinal funcs
		if(oft->u1.Ordinal & IMAGE_ORDINAL_FLAG)
		{
			continue;
		}

		// we get the function name then compare it against the parameter
		const auto funcName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(FROM_RVA(oft->u1.AddressOfData));
		if(!_stricmp(funcName->Name, functionName))
		{
#if _DEBUG
			printf_s("Found function '%s' at %p\n", functionName, firstThunk->u1.Function);
#endif

			// ptrToFunc is basically the pointer we need to modify when hooking and unhooking
			ptrToFunc = &firstThunk->u1.Function;

			// we return the deref of that pointer aka the function
			return *ptrToFunc;
		}
	}

	return 0;
}

bool IATHook::Hook()
{
	if (!canBeHooked)
	{
		return false;
	}

	// we set the pointer to our detour
	*ptrToFunc = detourFunc;

#if _DEBUG
	printf_s("Hooked function: % p\n", detourFunc);
#endif

	isHooked = true;
	return true;
}

bool IATHook::Unhook()
{
	if(!isHooked)
	{
		return false;
	}

	// we set the pointer back its original value
	*ptrToFunc = originalFunc;
	isHooked = false;
	return true;
}