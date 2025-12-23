#include "../../include/Services/ApiService.h"
#include "../../include/Services/LoginService.h"
#include <curl/curl.h>
#include <openssl/sha.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <memory>
#include <optional>
#include <functional>

// Helper function declarations outside namespace
static std::string extractJsonValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\":\"";
    size_t pos = json.find(searchKey);
    if (pos == std::string::npos) return "";
    
    pos += searchKey.length();
    size_t endPos = json.find("\"", pos);
    if (endPos == std::string::npos) return "";
    
    return json.substr(pos, endPos - pos);
}

namespace Services {

// Static member initialization
std::string ApiService::_baseUrl = "http://127.0.0.1:5000/api";
std::string ApiService::_authUsername;
std::string ApiService::_authPassword;

// Callback für CURL Response-Daten
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
{
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Progress Callback für Upload
static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                           curl_off_t ultotal, curl_off_t ulnow)
{
    if (ultotal > 0) {
        double progress = static_cast<double>(ulnow) / static_cast<double>(ultotal);
        auto callback = static_cast<std::function<void(double)>*>(clientp);
        if (callback) {
            (*callback)(progress);
        }
    }
    return 0;
}

void ApiService::Initialize(const std::string& backendIp, int port)
{
    _baseUrl = "http://" + backendIp + ":" + std::to_string(port) + "/api";
    std::cout << "ApiService initialisiert: " << _baseUrl << std::endl;
    
    // Lade gespeicherte Credentials vom LoginService
    auto credentials = LoginService::GetStoredCredentials();
    if (!credentials.first.empty()) {
        std::cout << "DEBUG: Credentials vom LoginService geladen für User: " << credentials.first << std::endl;
        SetAuthCredentials(credentials.first, credentials.second);
    }
}

void ApiService::SetApiUrl(const std::string& url)
{
    _baseUrl = url;
    if (_baseUrl.back() == '/') {
        _baseUrl.pop_back();
    }
    std::cout << "API-URL gesetzt: " << _baseUrl << std::endl;
}

void ApiService::SetApiUrl(const std::string& ip, int port)
{
    SetApiUrl("http://" + ip + ":" + std::to_string(port) + "/api");
}

std::string ApiService::GetApiUrl()
{
    return _baseUrl;
}

bool ApiService::CheckConnection(int timeoutSeconds)
{
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    try {
        curl_easy_setopt(curl, CURLOPT_URL, _baseUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            std::cout << "Verbindung zu " << _baseUrl << " erfolgreich" << std::endl;
            return true;
        }
        
        std::cout << "Verbindungsfehler: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    catch (...) {
        curl_easy_cleanup(curl);
        return false;
    }
}

HttpResponse ApiService::Get(const std::string& endpoint)
{
    CURL* curl = curl_easy_init();
    HttpResponse response;

    if (!curl) {
        response.statusCode = 0;
        response.body = "Failed to initialize CURL";
        return response;
    }

    std::string readBuffer;
    std::string url = _baseUrl + "/" + endpoint;
    std::string authHeader = GetAuthHeader();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!authHeader.empty()) {
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        response.statusCode = httpCode;
        response.body = readBuffer;
        response.isSuccess = (httpCode >= 200 && httpCode < 300);
    } else {
        response.statusCode = 0;
        response.body = std::string("CURL Error: ") + curl_easy_strerror(res);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    //std::cout << "GET " << url << " -> " << response.statusCode << std::endl;
    return response;
}

HttpResponse ApiService::Post(const std::string& endpoint, const std::string& jsonBody)
{
    CURL* curl = curl_easy_init();
    HttpResponse response;

    if (!curl) {
        response.statusCode = 0;
        response.body = "Failed to initialize CURL";
        return response;
    }

    std::string readBuffer;
    std::string url = _baseUrl + "/" + endpoint;
    std::string authHeader = GetAuthHeader();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!authHeader.empty()) {
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        response.statusCode = httpCode;
        response.body = readBuffer;
        response.isSuccess = (httpCode >= 200 && httpCode < 300);
    } else {
        response.statusCode = 0;
        response.body = std::string("CURL Error: ") + curl_easy_strerror(res);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    //std::cout << "POST " << url << " -> " << response.statusCode << std::endl;
    return response;
}

HttpResponse ApiService::Put(const std::string& endpoint, const std::string& jsonBody)
{
    CURL* curl = curl_easy_init();
    HttpResponse response;

    if (!curl) {
        response.statusCode = 0;
        response.body = "Failed to initialize CURL";
        return response;
    }

    std::string readBuffer;
    std::string url = _baseUrl + "/" + endpoint;
    std::string authHeader = GetAuthHeader();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!authHeader.empty()) {
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        response.statusCode = httpCode;
        response.body = readBuffer;
        response.isSuccess = (httpCode >= 200 && httpCode < 300);
    } else {
        response.statusCode = 0;
        response.body = std::string("CURL Error: ") + curl_easy_strerror(res);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    //std::cout << "PUT " << url << " -> " << response.statusCode << std::endl;
    return response;
}

HttpResponse ApiService::Delete(const std::string& endpoint)
{
    CURL* curl = curl_easy_init();
    HttpResponse response;

    if (!curl) {
        response.statusCode = 0;
        response.body = "Failed to initialize CURL";
        return response;
    }

    std::string readBuffer;
    std::string url = _baseUrl + "/" + endpoint;
    std::string authHeader = GetAuthHeader();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!authHeader.empty()) {
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        response.statusCode = httpCode;
        response.body = readBuffer;
        response.isSuccess = (httpCode >= 200 && httpCode < 300);
    } else {
        response.statusCode = 0;
        response.body = std::string("CURL Error: ") + curl_easy_strerror(res);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    //std::cout << "DELETE " << url << " -> " << response.statusCode << std::endl;
    return response;
}

HttpResponse ApiService::Login(const std::string& username, const std::string& password)
{
    std::string hashedPassword = HashPassword(password);
    std::string endpoint = "User?username=" + username + "&password=" + hashedPassword;
    
    HttpResponse resp = Get(endpoint);
    
    // Parse die Response um Email und Role zu extrahieren
    if (resp.isSuccess && !resp.body.empty()) {
        ParseLoginResponse(resp);
    }
    
    return resp;
}

HttpResponse ApiService::UploadFile(const std::string& filePath, 
                                    std::function<void(double)> progressCallback)
{
    std::cout << "UploadFile called with path: [" << filePath << "]" << std::endl;
    
    CURL* curl = curl_easy_init();
    HttpResponse response;

    if (!curl) {
        response.statusCode = 0;
        response.body = "Failed to initialize CURL";
        return response;
    }

    // Überprüfe ob Datei existiert
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp) {
        std::cout << "ERROR: Cannot open file: " << filePath << std::endl;
        response.statusCode = 0;
        response.body = "Failed to open file";
        curl_easy_cleanup(curl);
        return response;
    }
    fclose(fp);

    std::string readBuffer;
    std::string url = _baseUrl + "/Upload";
    std::string authHeader = GetAuthHeader();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: multipart/form-data");
    if (!authHeader.empty()) {
        std::cout << "DEBUG: Adding auth header to upload request" << std::endl;
        headers = curl_slist_append(headers, authHeader.c_str());
    } else {
        std::cout << "WARNING: No auth header available for upload!" << std::endl;
    }

    // Erstelle MIME-Post für multipart Datei-Upload
    curl_mime* mime = curl_mime_init(curl);
    curl_mimepart* part = curl_mime_addpart(mime);
    curl_mime_name(part, "file");
    curl_mime_filedata(part, filePath.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); // 5 Minuten für große Dateien

    if (progressCallback) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progressCallback);
    }

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        response.statusCode = httpCode;
        response.body = readBuffer;
        response.isSuccess = (httpCode >= 200 && httpCode < 300);
    } else {
        response.statusCode = 0;
        response.body = std::string("CURL Error: ") + curl_easy_strerror(res);
    }

    curl_mime_free(mime);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    std::cout << "UPLOAD " << filePath << " -> " << response.statusCode << std::endl;
    if (!response.body.empty()) {
        std::cout << "SERVER RESPONSE: " << response.body << std::endl;
    }
    return response;
}

void ApiService::SetAuthCredentials(const std::string& username, const std::string& password)
{
    _authUsername = username;
    _authPassword = password;
    std::cout << "Auth-Credentials gesetzt für: " << username << std::endl;
}

void ApiService::ClearAuthCredentials()
{
    _authUsername.clear();
    _authPassword.clear();
}

std::string ApiService::GetAuthHeader()
{
    if (_authUsername.empty() || _authPassword.empty()) {
        std::cout << "DEBUG: GetAuthHeader() - keine Credentials gespeichert" << std::endl;
        return "";
    }

    std::cout << "DEBUG: GetAuthHeader() - Username: " << _authUsername << ", Password (gehashed): " << _authPassword.substr(0, 8) << "..." << std::endl;

    std::string credentials = _authUsername + ":" + _authPassword;
    // Base64 encoding (vereinfacht - für Production sollte man eine Library verwenden)
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string encoded;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (size_t j = 0; j < credentials.length(); j++) {
        char_array_3[i++] = credentials[j];
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                encoded += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (int j = 0; j <= i; j++)
            encoded += base64_chars[char_array_4[j]];

        while (i++ < 3)
            encoded += '=';
    }

    return "Authorization: Basic " + encoded;
}

std::string ApiService::HashPassword(const std::string& password)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.length(), hash);
    return BytesToHex(hash, SHA256_DIGEST_LENGTH);
}

std::string ApiService::BytesToHex(const unsigned char* bytes, size_t length)
{
    std::stringstream ss;
    for (size_t i = 0; i < length; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
    }
    return ss.str();
}

void ApiService::ParseLoginResponse(HttpResponse& response)
{
    response.user_email = extractJsonValue(response.body, "email");
    response.user_role = extractJsonValue(response.body, "role");
    response.user_token = extractJsonValue(response.body, "token");
}

} // namespace Services
