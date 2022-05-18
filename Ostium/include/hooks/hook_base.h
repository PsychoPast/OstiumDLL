#pragma once

class HookBase
{

public:

    HookBase(uintptr_t original, uintptr_t detour)
	:
	originalFunc(original),
	detourFunc(detour)
    {
	    
    }

	[[nodiscard]] uintptr_t GetOriginalFunc() const { return originalFunc; }
	[[nodiscard]] uintptr_t GetDetourFunc() const { return detourFunc; }
	virtual bool Hook() = 0;
    virtual bool Unhook() = 0;

protected:

    uintptr_t originalFunc;
    uintptr_t detourFunc;
    bool isHooked;
    bool canBeHooked;
};