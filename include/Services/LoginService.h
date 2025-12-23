#pragma once

#include <string>
#include <optional>
#include <memory>
#include <functional>
#include <vector>

namespace Services {

/// <summary>
/// Login-Informationen DTO
/// </summary>
struct LoginInfo {
    std::string username;
    std::string email;
    std::string role = "User";
    std::string lastLogin;
};

/// <summary>
/// Login Service für Authentifizierung und Session-Verwaltung
/// Basierend auf C# LoginService Implementierung
/// </summary>
class LoginService {
public:
    /// <summary>
    /// Event-Typ für Login-Status-Änderungen
    /// </summary>
    using LoginStatusChangedEvent = std::function<void(bool)>;
    
    /// <summary>
    /// Initialisiert den Login Service
    /// </summary>
    static void Initialize();
    
    /// <summary>
    /// Prüft ob Benutzer eingeloggt ist
    /// </summary>
    static bool IsLoggedIn();
    
    /// <summary>
    /// Holt den aktuellen Benutzernamen
    /// </summary>
    static std::string GetUsername();
    
    /// <summary>
    /// Holt alle Login-Informationen
    /// </summary>
    static std::optional<LoginInfo> GetLoginInfo();
    
    /// <summary>
    /// Holt die gespeicherten Credentials (username und hashed password)
    /// </summary>
    static std::pair<std::string, std::string> GetStoredCredentials();
    
    /// <summary>
    /// Speichert Login-Informationen lokal
    /// </summary>
    static void SaveLogin(const std::string& username, 
                         const std::string& email = "",
                         const std::string& role = "User",
                         const std::string& password = "");
    
    /// <summary>
    /// Löscht alle Login-Informationen (Logout)
    /// </summary>
    static void ClearLogin();
    
    /// <summary>
    /// Aktualisiert den Last-Login Zeitstempel
    /// </summary>
    static void UpdateLastLogin();
    
    /// <summary>
    /// Registriert einen Event-Handler für Login-Status-Änderungen
    /// </summary>
    static void OnLoginStatusChanged(LoginStatusChangedEvent handler);
    
private:
    static bool _isLoggedIn;
    static std::string _username;
    static std::string _email;
    static std::string _role;
    static std::string _password;
    static std::string _lastLogin;
    static LoginStatusChangedEvent _statusChangeHandler;
    
    // Speicherpfade für persistente Daten (optional)
    static std::string GetStoragePath();
    static void LoadFromStorage();
    static void SaveToStorage();
    
    // Verschlüsselung für sichere Speicherung
    static std::string Encrypt(const std::string& plaintext);
    static std::string Decrypt(const std::string& ciphertext);
};

} // namespace Services
