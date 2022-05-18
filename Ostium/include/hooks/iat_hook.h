#pragma once

#include "hook_base.h"

// in a PE file, all the items' address is relative to the base address. therefore, to obtain the real address, we have to add the imageBaseAddress to the item RVA (relative virtual address)
#define FROM_RVA(x) (_imageBaseAddress + (x))

class IATHook
    : public HookBase
{

public:

    IATHook(const char* moduleName, const char* functionName, uintptr_t detour);
    bool Hook() override;
    bool Unhook() override;

private:
    
	const PBYTE _imageBaseAddress = reinterpret_cast<PBYTE>(GetModuleHandle(nullptr));
    static PIMAGE_IMPORT_DESCRIPTOR _importDirectoryTable;

    uintptr_t* ptrToFunc;

    PIMAGE_IMPORT_DESCRIPTOR GetImportDirectoryTable();
    IMAGE_IMPORT_DESCRIPTOR* GetModule(const char* moduleName);
    uintptr_t GetFunctionFromModule(IMAGE_IMPORT_DESCRIPTOR module, const char* functionName);
    uintptr_t GetFunctionFromModule(const char* moduleName, const char* functionName);
    bool EnablePageWriting(void* address);
};