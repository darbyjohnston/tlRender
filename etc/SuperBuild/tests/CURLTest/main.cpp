#include <curl/curl.h>

int main(int argc, char* argv[])
{
    CURL *curl = curl_easy_init();
    curl_easy_cleanup(curl);
    return 0;
}
