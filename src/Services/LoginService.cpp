#include "../../include/Services/LoginService.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>

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
    std::cout << "✓ LoginService initialisiert" << std::endl;
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

    std::cout << "✓ Login gespeichert für: " << username << std::endl;

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

    std::cout << "✓ Login gelöscht" << std::endl;

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
    // Vereinfacht: speichert im aktuellen Verzeichnis als .login
    return ".login";
}

void LoginService::LoadFromStorage()
{
    std::ifstream file(GetStoragePath());
    if (!file.is_open()) {
        _isLoggedIn = false;
        return;
    }

    try {
        std::getline(file, _username);
        std::getline(file, _email);
        std::getline(file, _role);
        std::getline(file, _password);
        std::getline(file, _lastLogin);

        _isLoggedIn = !_username.empty();

        if (_isLoggedIn) {
            std::cout << "✓ Login-Informationen vom Speicher geladen für: " << _username << std::endl;
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
    std::ofstream file(GetStoragePath());
    if (!file.is_open()) {
        std::cerr << "Fehler: Konnte Login-Datei nicht öffnen zum Speichern" << std::endl;
        return;
    }

    try {
        file << _username << "\n";
        file << _email << "\n";
        file << _role << "\n";
        file << _password << "\n";
        file << _lastLogin << "\n";
        file.flush();
    }
    catch (const std::exception& ex) {
        std::cerr << "Fehler beim Speichern der Login-Informationen: " << ex.what() << std::endl;
    }

    file.close();
}

} // namespace Services
