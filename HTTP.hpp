#pragma once
#include <string>
#include <curl/curl.h>

namespace HTTP
{
	extern std::string contentType;

	extern bool init();
	extern void cleanup();
	extern char* GET(std::string URL, size_t* bytesRead = nullptr); // you are responsible for free()ing
	extern char* POST(std::string URL, size_t* bytesRead = nullptr, char* postData = nullptr, size_t postDataLength = 0); // you are responsible for free()ing
}