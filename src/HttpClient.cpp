#include "HttpClient.hpp"

// Handle API responses
size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* output) {
  size_t totalSize = size * nmemb;
  output->append((char*)contents, totalSize);
  return totalSize;
}

// Send HTTP requests
std::string HttpClient::send_request(const std::string& url, const std::string& post_data, const std::string& token) {
  CURL* curl = curl_easy_init();
  std::string response;

  if (curl) {
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Add Authorization header if a token is provided
    if (!token.empty()) {
      std::string auth_header = "Authorization: Bearer " + token;
      headers = curl_slist_append(headers, auth_header.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Handle POST requests
    if (!post_data.empty()) {
      curl_easy_setopt(curl, CURLOPT_POST, 1);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      std::cerr << "❌ CURL Error: " << curl_easy_strerror(res) << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  }

  return response;
}
