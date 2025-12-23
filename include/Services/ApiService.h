#pragma once

#include <string>
#include <memory>
#include <map>
#include <functional>

namespace Services {

/// <summary>
/// HTTP Response wrapper
/// </summary>
struct HttpResponse {
    int statusCode = 0;
    std::string body;
    std::map<std::string, std::string> headers;
    bool isSuccess = false;
    
    // Login-Daten aus JSON-Response
    std::string user_email;
    std::string user_role;
    std::string user_token;
};

/// <summary>
/// API Service für REST API Kommunikation
/// Basierend auf C# ApiService Implementierung
/// </summary>
class ApiService {
public:
    /// <summary>
    /// Initialisiert den API Service mit Backend-Adresse
    /// </summary>
    static void Initialize(const std::string& backendIp = "127.0.0.1", int port = 5000);
    
    /// <summary>
    /// Setzt die API-URL manuell
    /// </summary>
    static void SetApiUrl(const std::string& url);
    
    /// <summary>
    /// Setzt API-URL basierend auf IP und Port
    /// </summary>
    static void SetApiUrl(const std::string& ip, int port);
    
    /// <summary>
    /// Gibt die aktuelle API-URL zurück
    /// </summary>
    static std::string GetApiUrl();
    
    /// <summary>
    /// Prüft die Verbindung zum Backend
    /// </summary>
    static bool CheckConnection(int timeoutSeconds = 5);
    
    // HTTP Methods
    static HttpResponse Get(const std::string& endpoint);
    static HttpResponse Post(const std::string& endpoint, const std::string& jsonBody);
    static HttpResponse Put(const std::string& endpoint, const std::string& jsonBody);
    static HttpResponse Delete(const std::string& endpoint);
    
    /// <summary>
    /// Login mit Benutzername und Passwort
    /// </summary>
    static HttpResponse Login(const std::string& username, const std::string& password);
    
    /// <summary>
    /// Datei-Upload mit optional Progress-Callback
    /// </summary>
    static HttpResponse UploadFile(const std::string& filePath, 
                                   std::function<void(double)> progressCallback = nullptr);
    
    /// <summary>
    /// Setzt den Authorization Header für nachfolgende Requests
    /// </summary>
    static void SetAuthCredentials(const std::string& username, const std::string& password);
    
    /// <summary>
    /// Löscht die Authorization Header
    /// </summary>
    static void ClearAuthCredentials();
    
private:
    static std::string _baseUrl;
    static std::string _authUsername;
    static std::string _authPassword;
    
    /// <summary>
    /// Erstellt einen Authorization Header (Basic Auth)
    /// </summary>
    static std::string GetAuthHeader();
    
    /// <summary>
    /// SHA256-Hash für Passwort
    /// </summary>
    static std::string HashPassword(const std::string& password);
    
    /// <summary>
    /// Konvertiert Bytes zu Hex-String
    /// </summary>
    static std::string BytesToHex(const unsigned char* bytes, size_t length);
    
    /// <summary>
    /// Parst JSON Response für Login-Daten
    /// </summary>
    static void ParseLoginResponse(HttpResponse& response);
};
} // namespace Services