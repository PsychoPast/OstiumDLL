#include "pch.h"
#include "hooks/veh_hook.h"
#include <cstdio>

std::vector<VEHHook*> VEHHook::_activeHooks{};

bool VEHHook::Hook()
{
	if((_vehHandle = AddVectoredExceptionHandler(1, &VectoredExceptionHandler)))
	{
#if _DEBUG
		printf_s("Added new vectored exception: %p\n", _vehHandle);
#endif

		_activeHooks.push_back(this);
		return true;
	}

	return false;
}

bool VEHHook::Unhook()
{
	if(_vehHandle)
	{
		if(RemoveVectoredExceptionHandler(_vehHandle))
		{
#if _DEBUG
			printf_s("Removed vectored exception: %p\n", _vehHandle);
#endif

			return true;
		}
	}

	return false;
}

LONG VEHHook::VectoredExceptionHandler(_EXCEPTION_POINTERS* exceptionInfo)
{
	switch(exceptionInfo->ExceptionRecord->ExceptionCode)
	{
	    case EXCEPTION_GUARD_PAGE:
		{
		   const auto nextAddress = &exceptionInfo->ContextRecord->Rip;
			if(const auto hookedFunction = std::find_if(_activeHooks.begin(), _activeHooks.end(), [nextAddress](VEHHook*& hook) { return hook->originalFunc == *nextAddress; }); hookedFunction != _activeHooks.end())
			{
				// first, we get the VEHHook instance. hookedFunction.Ptr is a pointer to pointer (since our vector is a VEHHook* container)
				// therefore we dereference it to obtain our instance
				// next we check if the instance has a contionalHook set. If it doesn't or if it does and it returns true, we proceed with our hook
				if (VEHHook* instance = *hookedFunction._Ptr; !instance->conditionalHook || instance->conditionalHook(exceptionInfo->ContextRecord))
				{
					// if we successfuly got it, we set 'nextAddress' (RIP value) to our detour
					*nextAddress = instance->detourFunc;

					// if we hit the function '_maxHits' times, we automatically unhook it, remove the instance from the vector and delete it
					if ((++instance->_hitCount == instance->_maxHits) || (instance->conditionalUnhook && instance->conditionalUnhook(exceptionInfo->ContextRecord)))
					{
						// remove the veh handler
						instance->Unhook();

						// remove the instance from the vector
						_activeHooks.erase(hookedFunction);

						// delete the instance (made with new() thus resides on the heap and needs to be freed manually)
						delete instance;
					}
					else
					{
						// we set the trap flag in order to trigger a EXCEPTION_SINGLE_STEP
						exceptionInfo->ContextRecord->EFlags |= 0x100; // (1 << 8)  we're settings the 8th bit (Trap Flag) of EFlags to 1
					}
				}
			}

			return EXCEPTION_CONTINUE_EXECUTION;
		}

		case EXCEPTION_SINGLE_STEP:
		{
			DWORD meh;
			VirtualProtect(exceptionInfo->ExceptionRecord->ExceptionAddress, 1, PAGE_GUARD, &meh);
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		default:
			return EXCEPTION_CONTINUE_SEARCH;
	}
}