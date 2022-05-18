#pragma once

#include "../libs/libwebsockets/include/libwebsockets.h"
#include "../libs/libcurl/curl.h"

inline decltype(lws_client_connect_via_info)* lwsClientConnectViaInfo_;
inline decltype(lws_create_context)* lwsCreateContext_;
inline decltype(curl_easy_setopt)* curlEasySetopt_;
inline CURLcode(*curlSetopt_)(CURL*, CURLoption, va_list);

class SSLManager
{
public:

	bool PatchHTTP();
	bool PatchWSS();

private:

	static lws* LwsClientConnectViaInfoDetour(lws_client_connect_info* ccinfo);
	static lws_context* LwsCcreateCcontextDetour(struct lws_context_creation_info* info);
	static CURLcode CurlEasySetoptDetour(CURL* curl, CURLoption option, ...);
	static CURLcode CurlSetoptDetour(CURL* curl, CURLoption option, ...);

};