#include "pch.h"
#include "ssl_manager.h"
#include <cstdio>
#include <cstdlib>

#include "patterns.h"
#include "util.h"
#include "hooks/veh_hook.h"

lws_context* SSLManager::LwsCcreateCcontextDetour(lws_context_creation_info* info)
{
	if(info->options & LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT)
	{
		info->options &= ~LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	}

	// if options doesn't contain 'LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT' and 'provided_client_ssl_ctx' it simply won't use ssl
	info->provided_client_ssl_ctx = nullptr;
	return lwsCreateContext_(info);
}

lws* SSLManager::LwsClientConnectViaInfoDetour(lws_client_connect_info* ccinfo)
{
	ccinfo->port = 80;
	ccinfo->ssl_connection = 0;
	ccinfo->protocol = "ws";
	ccinfo->host = ccinfo->origin = "";
	return lwsClientConnectViaInfo_(ccinfo);
}

// base copied from https://github.com/curl/curl/blob/8839c05fba1f8415eec17eff8ac60cc3a50eb51e/lib/easy.c#L374 and modified to fit our needs
CURLcode SSLManager::CurlEasySetoptDetour(CURL* curl, CURLoption option, ...)
{
	if (!curl)
		return CURLE_BAD_FUNCTION_ARGUMENT;

	va_list arg;
	// at runtime, arg will point to the start of the variadic parameters
	va_start(arg, option);

	// if the game is setting the SSL_VERIFYPEER option, we intercept it and change its value to false (0) before calling curl_setopt
	if(option == CURLOPT_SSL_VERIFYPEER)
	{
		// we get the address of the first va arg (libcurl docs clearly states that for CURLOPT_SSL_VERIFYPEER, this function takes only one va arg (typeof(long)), then we dereference it and sets the new value (0)
		*&va_arg(arg, long) = 0;

		// that's a custom macro. basically va_arg increments the pointer of arg. therefore, if curl_setopt will try to call it after we did, it will fail to get the value. So va_back simply sets arg back to its original value
		va_back(arg);

		// this also works but I like the other way more
		// *reinterpret_cast<long*>(arg) = 0;
	}

	// we call the original function 
	const CURLcode result = curlSetopt_(curl, option, arg);
	va_end(arg);
	return result;
}

bool SSLManager::PatchHTTP()
{
	// get the address of curl_setopt
	{
		const auto curlSetoptRelPattern = findPattern(mainModuleBaseAddress, mainModuleSize, Patterns::CurlSetopt.pattern, Patterns::CurlSetopt.mask, Patterns::CurlSetopt.size);
		if (!curlSetoptRelPattern)
		{
			printf_s("Couldn't find curl_setopt, exiting...");
			return false;
		}

		curlSetopt_ = reinterpret_cast<decltype(curlSetopt_)>(curlSetoptRelPattern);
#if _DEBUG
		printf_s("curl_setopt address: %p,\n", curlSetoptRelPattern);
#endif

		const auto hook = new VEHHook(reinterpret_cast<uintptr_t>(curlSetoptRelPattern), reinterpret_cast<uintptr_t>(&CurlSetoptDetour));
		hook->conditionalHook = [](const PCONTEXT context) { return context->Rdx == CURLOPT_SSL_VERIFYPEER; };
		hook->Hook();
	}

	// get the address of curl_easy_setopt
	{
		const auto curlEasySetoptRelPattern = findPattern(mainModuleBaseAddress, mainModuleSize, Patterns::CurlEasySetopt.pattern, Patterns::CurlEasySetopt.mask, Patterns::CurlEasySetopt.size);
		if (!curlEasySetoptRelPattern)
		{
			printf_s("Couldn't find curl_easy_setopt, exiting...");
			return false;
		}

		const auto curlEasySetopt = *reinterpret_cast<uintptr_t*>((curlEasySetoptRelPattern + 5)/*RIP*/ + *reinterpret_cast<int*>(curlEasySetoptRelPattern + 1));
		curlEasySetopt_ = reinterpret_cast<decltype(curlEasySetopt_)>(curlEasySetopt);
#if _DEBUG
		printf_s("curl_easy_setopt address: %p,\n", curlEasySetopt);
#endif

		const auto hook = new VEHHook(reinterpret_cast<uintptr_t>(curlEasySetopt), reinterpret_cast<uintptr_t>(&CurlEasySetoptDetour));
		hook->conditionalHook = [](const PCONTEXT context) { return context->Rdx == CURLOPT_SSL_VERIFYPEER; };
		hook->Hook();
	}

	return true;
}

bool SSLManager::PatchWSS()
{
	// we get the address of lws_create_context
	{
		const auto lwsCreateContextRelPattern = findPattern(mainModuleBaseAddress, mainModuleSize, Patterns::LwsCreateContext.pattern, Patterns::LwsCreateContext.mask, Patterns::LwsCreateContext.size);
		if (!lwsCreateContextRelPattern)
		{
			printf_s("Couldn't find lws_create_context, exiting...");
			return false;
		}

		const auto lwsCreateContext = *reinterpret_cast<uintptr_t*>((lwsCreateContextRelPattern + 5)/*RIP*/ + *reinterpret_cast<int*>(lwsCreateContextRelPattern + 1));
		lwsCreateContext_ = reinterpret_cast<decltype(lwsCreateContext_)>(lwsCreateContext);

#if _DEBUG
		printf_s("lws_create_context address: %p,\n", lwsCreateContext);
#endif

		const auto hook = new VEHHook(reinterpret_cast<uintptr_t>(lwsCreateContext), reinterpret_cast<uintptr_t>(&LwsCcreateCcontextDetour), 1);
		hook->Hook();
	}

	// we get the address of lws_client_connect_via_info
	{
		const auto lwsClientConnectViaInfoPattern = findPattern(mainModuleBaseAddress, mainModuleSize, Patterns::LwsClientConnect.pattern, Patterns::LwsClientConnect.mask, Patterns::LwsClientConnect.size);
		if (!lwsClientConnectViaInfoPattern)
		{
			printf_s("Couldn't find lws_client_connect_via_info, exiting...");
			return false;
		}

		lwsClientConnectViaInfo_ = reinterpret_cast<decltype(lwsClientConnectViaInfo_)>(lwsClientConnectViaInfoPattern);

#if _DEBUG
		printf_s("lws_client_connect_via_info address: %p\n", lwsClientConnectViaInfoPattern);
#endif

		const auto hook = new VEHHook(reinterpret_cast<uintptr_t>(lwsClientConnectViaInfoPattern), reinterpret_cast<uintptr_t>(&LwsClientConnectViaInfoDetour), 1);
		hook->Hook();
	}

	return true;
}