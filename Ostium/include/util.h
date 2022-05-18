#pragma once

#include "framework.h"

#define va_back(ap) (((ap) -= sizeof(__int64)) + sizeof(__int64))

inline uintptr_t mainModuleBaseAddress;
inline DWORD mainModuleSize;

void attachConsoleToProcess();
bool getAddressAndSizeOfModule(const char* moduleName, uintptr_t& moduleAddress, DWORD& moduleSize);
bool iterateThroughPattern(PBYTE currentAddress, const char* pattern, const char* mask, DWORD size);
PBYTE findPattern(uintptr_t baseAddress, DWORD size, const char* pattern, const char* mask, DWORD patternSize);