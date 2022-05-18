#pragma once

#include <cstdint>
#include <locale>
#include <cstdio>

typedef void* FVtable;

struct FNameEntryId //sizeof => 4 (0x4)
{
	uint32_t value; //0x0
};

struct FName //sizeof => 8 (0x8)
{
	FNameEntryId comparisonIndex; //0x0
	uint32_t number; //0x4
};

template<class T>
struct TArray
{

protected:
	T* data;
	int arrayNum;
	int arrayMax;
};

struct FString : TArray<WCHAR>
{
	explicit FString(const wchar_t* other) : TArray()
	{
		arrayMax = arrayNum = std::wcslen(other) + 1;
		data = const_cast<wchar_t*>(other);
	}
};

struct UClass;

struct UObject //sizeof => 40 (0x28)
{
	FVtable vtablePtrBase; //0x0
	uint32_t objectFlags; //0x8
	int32_t internalIndex; //0xC
	UClass* classPrivate; //0x10
	FName namePrivate; //0x18
	UObject* outerPrivate; //0x20
};

struct UConsole : UObject
{
	void ConsoleCommand(const FString command)
	{
		const auto virtualTable = static_cast<void**>(vtablePtrBase);
		const auto consoleCommand = static_cast<void(*)(void*, FString)>(virtualTable[0x50]);
		if(!consoleCommand)
		{
			printf_s("Couldn't find console command address, exiting...");
			exit(69);
		}

#if _DEBUG
		printf_s("UConsole::ConsoleCommand address: %p\n", consoleCommand);
#endif
		consoleCommand(this, command);
	}
};

struct UField : UObject
{

};

struct UStruct : UField
{
	
};

struct UClass : UStruct
{
	
};

struct UGameViewportClient : UObject
{
	FVtable vtablePtr; //0x28
	char notNeeded0[0x10];
	UConsole* viewPortConsole; //0x40
};

struct UEngine : UObject
{
	FVtable vtablePtr; //0x28
	char notNeeded0[0xC0];
	UClass* consoleClass; //0xF0
	char notNeeded1[0x688];
	UGameViewportClient* gameViewport; //0x780
};

struct FStaticConstructObjectParameters //sizeof => 58
{
	UClass* uclass; //0x0
	void* outer; //0x8
	FName name; //0x10
	uint32_t setflags; //0x18
	uint32_t internalsetflags; //0x1C
	bool bCopyTransientsFromClassDefaults; //0x20
	bool bAssumeTemplateIsArchetype; //0x21
	UObject* utemplate; //0x22
	/*FObjectInstancingGraph*/ void* instanceGraph; //0x2A
	/*UPackage*/ void* externalPackage; //0x32
};