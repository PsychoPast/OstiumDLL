#pragma once

#include "pch.h"

int64_t __fastcall Equ8ClientInitializeDetour(char* clientPath, DWORD operatingMode)
{
	// the game checks if equ8_client_initialize returned 0. If it didn't, it means that something went wrong thus terminates the process.
	// by returning 0, we're telling the game that everything went okay
	return 0;
}

int64_t __fastcall Equ8ClientPollEventDetour(void* data)
{
	return 0;
}