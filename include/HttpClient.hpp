#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <curl/curl.h>

#include <iostream>
#include <string>

class HttpClient {
 public:
  std::string send_request(const std::string& url, const std::string& post_data = "", const std::string& token = "");
};

#endif
