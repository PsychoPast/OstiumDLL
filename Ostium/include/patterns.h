#pragma once

struct Sig
{
	const char* pattern;
	const char* mask;
	const int size;
};

class Patterns
{
public:

	// direct
	static constexpr Sig Gengine { "\x48\x89\x05\x00\x00\x00\x00\x48\x85\xC9\x74\x00\xE8\x00\x00\x00\x00\x48\x8D\x4D\x00", "xxx????xxxx?x????xxx?", 21 };

	// direct
    static constexpr Sig Soci { "\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x55\x57\x41\x54\x41\x56\x41\x57\x48\x8D\xAC\x24\x00\x00\x00\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x85\x00\x00\x00\x00\x48\x8B\x39", "xxxx?xxxx?xxxxxxxxxxxx????xxx????xxx????xxxxxx????xxx", 53 };

	// call
	static constexpr Sig LwsCreateContext { "\xE8\x00\x00\x00\x00\x48\x89\x47\x20\x48\x85\xC0\x75\x34", "x????xxxxxxxxx", 15 };

	// direct
	static constexpr Sig LwsClientConnect {"\x48\x89\x74\x24\x10\x57\x48\x83\xEC\x20\x48\x8B\x71\x30", "xxxxxxxxxxxxxx", 15 };

	// direct
	static constexpr Sig CurlSetopt { "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x33\xED\x49\x8B\xF0\x48\x8B\xD9", "xxxx?xxxx?xxxx?xxxx?xxxxxxxx", 29 };

	// call
	static constexpr Sig CurlEasySetopt { "\xE8\x00\x00\x00\x00\x4C\x63\xB6\x80\x04\x00\x00", "x????xxxxxxx", 13 };
};