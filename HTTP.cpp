#include "HTTP.hpp"

namespace HTTP
{
    std::string contentType = "application/x-www-form-urlencoded";
}

#define SETCONTENTTYPE(curl, contentType) \
{ \
    struct curl_slist* __SETCONTENTTYPE_hs = NULL; \
    __SETCONTENTTYPE_hs = curl_slist_append(__SETCONTENTTYPE_hs, ("Content-Type: " + std::string(contentType)).c_str()); \
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, __SETCONTENTTYPE_hs); \
}

struct httpWriteData
{
    char* output;
    size_t outputSize;
    size_t outputBytesUsed;
};

size_t httpWriteCallback(void* contents, size_t __one, size_t nmemb, void* userp)
{
    httpWriteData* d = (httpWriteData*)userp;
    if (!d->output) return 0;

    // resize output
    if (d->outputBytesUsed + nmemb > d->outputSize)
    {
        void* re = realloc(d->output, d->outputBytesUsed + nmemb);
        if (!re)
        {
            free(d->output);
            return 0;
        }
        d->output = (char*)re;
    }

    // write to output
    memcpy(d->output + d->outputBytesUsed, contents, nmemb);
    d->outputBytesUsed += nmemb;

    return nmemb;
}

bool HTTP::init()
{
    return curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK;
}

void HTTP::cleanup()
{
    curl_global_cleanup();
}

char* HTTP::GET(std::string URL, size_t* bytesRead)
{
    CURL* c = curl_easy_init();
    if (!c) return nullptr;
    CURLcode err;

    httpWriteData d{};
    if (!(d.output = (char*)malloc(64))) return nullptr;
    d.outputSize = 64;
    d.outputBytesUsed = 0;

    SETCONTENTTYPE(c, HTTP::contentType);
    curl_easy_setopt(c, CURLOPT_URL, URL.c_str());
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, httpWriteCallback);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &d);
    err = curl_easy_perform(c);
    curl_easy_cleanup(c);

    if (err != CURLE_OK)
    {
        free(d.output);
        if (bytesRead) *bytesRead = 0;
        return nullptr;
    }
    if (d.output)
    {
        if (d.outputBytesUsed == 0)
        {
            // transferred empty file
            ZeroMemory(d.output, d.outputSize);
            if (bytesRead) *bytesRead = 0;
            return nullptr;
        }
        else if (d.outputBytesUsed < d.outputSize)
        {
            void* re = realloc(d.output, d.outputBytesUsed);
            if (!re)
            {
                free(d.output);
                if (bytesRead) *bytesRead = 0;
                return nullptr;
            }
            d.output = (char*)re;
            d.outputSize = d.outputBytesUsed;
        }
        if (bytesRead && d.output) *bytesRead = d.outputBytesUsed;
        return d.output;
    }
    if (bytesRead) *bytesRead = 0;
    return nullptr;
}
char* HTTP::POST(std::string URL, size_t* bytesRead, char* postData, size_t postDataLength)
{
    CURL* c = curl_easy_init();
    if (!c) return nullptr;
    CURLcode err;

    httpWriteData d{};
    if (!(d.output = (char*)malloc(64))) return nullptr;
    d.outputSize = 64;
    d.outputBytesUsed = 0;

    SETCONTENTTYPE(c, HTTP::contentType);
    curl_easy_setopt(c, CURLOPT_URL, URL.c_str());
    if (postData)
    {
        curl_easy_setopt(c, CURLOPT_POSTFIELDS, postData);
        curl_easy_setopt(c, CURLOPT_POSTFIELDSIZE, postDataLength);
    }
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, httpWriteCallback);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &d);
    err = curl_easy_perform(c);
    curl_easy_cleanup(c);

    if (err != CURLE_OK)
    {
        free(d.output);
        if (bytesRead) *bytesRead = 0;
        return nullptr;
    }
    if (d.output)
    {
        if (d.outputBytesUsed == 0)
        {
            // transferred empty file
            ZeroMemory(d.output, d.outputSize);
            if (bytesRead) *bytesRead = 0;
            return nullptr;
        }
        else if (d.outputBytesUsed < d.outputSize)
        {
            void* re = realloc(d.output, d.outputBytesUsed);
            if (!re)
            {
                free(d.output);
                if (bytesRead) *bytesRead = 0;
                return nullptr;
            }
            d.output = (char*)re;
            d.outputSize = d.outputBytesUsed;
        }
        if (bytesRead && d.output) *bytesRead = d.outputBytesUsed;
        return d.output;
    }
    if (bytesRead) *bytesRead = 0;
    return nullptr;
}