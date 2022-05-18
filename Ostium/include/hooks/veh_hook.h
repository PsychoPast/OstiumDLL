#pragma once

#include <functional>

#include "hook_base.h"
#include <vector>

struct VehFunction
{

};

class VEHHook
	: HookBase
{
public:

	std::function<bool(PCONTEXT)> conditionalHook;
	std::function<bool(PCONTEXT)> conditionalUnhook;

	VEHHook(const uintptr_t original, const uintptr_t detour, const DWORD hits)
	:
	HookBase(original, detour),
	_maxHits(hits)
	{
		
	}

	VEHHook(const uintptr_t original, const uintptr_t detour)
		:
		VEHHook(original, detour, -1)
	{
		
	}

	bool Hook() override;
	bool Unhook() override;

private:

	static std::vector<VEHHook*> _activeHooks;

	const int32_t _maxHits;
	PVOID _vehHandle;
	int32_t _hitCount;
	
	static LONG VectoredExceptionHandler(_EXCEPTION_POINTERS* exceptionInfo);
};