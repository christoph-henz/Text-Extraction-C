#include "../../include/Services/LoginService.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdio>

#ifndef _WIN32
    #include <sys/stat.h>
#endif

namespace Services {

// Static member initialization
bool LoginService::_isLoggedIn = false;
std::string LoginService::_username;
std::string LoginService::_email;
std::string LoginService::_role = "User";
std::string LoginService::_password;
std::string LoginService::_lastLogin;
LoginService::LoginStatusChangedEvent LoginService::_statusChangeHandler = nullptr;

void LoginService::Initialize()
{
    LoadFromStorage();
}

bool LoginService::IsLoggedIn()
{
    return _isLoggedIn && !_username.empty();
}

std::string LoginService::GetUsername()
{
    return _username;
}

std::optional<LoginInfo> LoginService::GetLoginInfo()
{
    if (!IsLoggedIn()) {
        return std::nullopt;
    }

    return LoginInfo{
        _username,
        _email,
        _role,
        _lastLogin
    };
}

std::pair<std::string, std::string> LoginService::GetStoredCredentials()
{
    return {_username, _password};
}

void LoginService::SaveLogin(const std::string& username,
                            const std::string& email,
                            const std::string& role,
                            const std::string& password)
{
    _username = username;
    _email = email;
    _role = role.empty() ? "User" : role;
    _password = password;
    _isLoggedIn = true;

    // Update last login timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    _lastLogin = std::ctime(&time_t);

    SaveToStorage();
    if (_statusChangeHandler) {
        _statusChangeHandler(true);
    }
}

void LoginService::ClearLogin()
{
    _isLoggedIn = false;
    _username.clear();
    _email.clear();
    _role = "User";
    _password.clear();
    _lastLogin.clear();

    SaveToStorage();

    if (_statusChangeHandler) {
        _statusChangeHandler(false);
    }
}

void LoginService::UpdateLastLogin()
{
    if (!IsLoggedIn()) {
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    _lastLogin = std::ctime(&time_t);

    SaveToStorage();
}

void LoginService::OnLoginStatusChanged(LoginStatusChangedEvent handler)
{
    _statusChangeHandler = handler;
}

std::string LoginService::GetStoragePath()
{
    // Speichert im Home-Verzeichnis für bessere Persistenz
    #ifdef _WIN32
        const char* home = std::getenv("USERPROFILE");
    #else
        const char* home = std::getenv("HOME");
    #endif
    
    if (!home) {
        return ".text-extraction-login";
    }
    
    std::string path = home;
    path += "/.text-extraction-login";
    return path;
}

// XOR-basierte einfache Verschlüsselung mit Schlüssel
std::string LoginService::Encrypt(const std::string& plaintext)
{
    static const unsigned char KEY[] = {0x42, 0xA7, 0x3F, 0xC9, 0x15, 0x8E, 0x7D, 0xB2};
    std::string result;
    
    for (size_t i = 0; i < plaintext.length(); ++i) {
        unsigned char encrypted = plaintext[i] ^ KEY[i % sizeof(KEY)];
        result += encrypted;
    }
    
    // Konvertiere zu Hex-String für Datenspeicherung
    std::stringstream ss;
    for (unsigned char c : result) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return ss.str();
}

std::string LoginService::Decrypt(const std::string& ciphertext)
{
    static const unsigned char KEY[] = {0x42, 0xA7, 0x3F, 0xC9, 0x15, 0x8E, 0x7D, 0xB2};
    std::string result;
    
    // Konvertiere von Hex-String
    for (size_t i = 0; i < ciphertext.length(); i += 2) {
        std::string byte = ciphertext.substr(i, 2);
        unsigned char c = (unsigned char)std::stoi(byte, nullptr, 16);
        result += (char)(c ^ KEY[(i / 2) % sizeof(KEY)]);
    }
    
    return result;
}

void LoginService::LoadFromStorage()
{
    std::string storagePath = GetStoragePath(); 
    std::ifstream file(storagePath);
    if (!file.is_open()) {
        std::cout << "ℹKeine gespeicherten Login-Daten gefunden" << std::endl;
        _isLoggedIn = false;
        return;
    }

    try {
        std::string encryptedUsername, encryptedEmail, encryptedRole, encryptedPassword, lastLogin;
        
        std::getline(file, encryptedUsername);
        std::getline(file, encryptedEmail);
        std::getline(file, encryptedRole);
        std::getline(file, encryptedPassword);
        std::getline(file, lastLogin);

        // Entschlüssele die Daten
        if (!encryptedUsername.empty()) {
            _username = Decrypt(encryptedUsername);
            _email = Decrypt(encryptedEmail);
            _role = Decrypt(encryptedRole);
            _password = Decrypt(encryptedPassword);
            _lastLogin = lastLogin;
            
            _isLoggedIn = !_username.empty();
        } else {
            _isLoggedIn = false;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Fehler beim Laden der Login-Informationen: " << ex.what() << std::endl;
        _isLoggedIn = false;
    }

    file.close();
}

void LoginService::SaveToStorage()
{
    std::string storagePath = GetStoragePath();
     
    std::ofstream file(storagePath);
    if (!file.is_open()) {
        std::cerr << "Fehler: Konnte Login-Datei nicht öffnen zum Speichern: " << storagePath << std::endl;
        return;
    }

    try {
        // Verschlüssele die sensiblen Daten
        file << Encrypt(_username) << "\n";
        file << Encrypt(_email) << "\n";
        file << Encrypt(_role) << "\n";
        file << Encrypt(_password) << "\n";
        file << _lastLogin << "\n";
        file.flush();
        
        // Setze Dateiberechtigungen auf nur Benutzer-lesbar (Unix/Linux)
        #ifndef _WIN32
            chmod(storagePath.c_str(), 0600); // rw-------
        #endif
}
    catch (const std::exception& ex) {
        std::cerr << "Fehler beim Speichern der Login-Informationen: " << ex.what() << std::endl;
    }

    file.close();
}

} // namespace Services
