#pragma once

#include "../ViewModel/MainViewModel.h"
#include "../../UI/Sidebar.h"
#include "../../Services/ApiService.h"
#include "../../Services/LoginService.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <fstream>
#include <codecvt>
#include <locale>
#include <thread>
#include <chrono>

#ifdef USE_SFML
#include <SFML/Graphics.hpp>
#endif

using namespace Presentation::ViewModel;

// UTF-8 zu UTF-32 Konvertierungsfunktion für SFML
inline sf::String ToSFMLString(const std::string& utf8String)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    try {
        std::u32string utf32 = converter.from_bytes(utf8String);
        // Konvertiere std::u32string zu std::basic_string<sf::Uint32>
        std::basic_string<sf::Uint32> sfUtf32(utf32.begin(), utf32.end());
        return sf::String(sfUtf32);
    }
    catch (...) {
        // Fallback bei Conversion-Fehler
        return sf::String(utf8String);
    }
}

// Extrahiere "message" Feld aus JSON Response
inline std::string ExtractMessageFromJSON(const std::string& json)
{
    size_t pos = json.find("\"message\":");
    if (pos == std::string::npos) {
        return json; // Fallback: ganze Response wenn kein message Feld
    }
    
    pos = json.find("\"", pos + 10);
    if (pos == std::string::npos) return json;
    
    size_t end = json.find("\"", pos + 1);
    if (end == std::string::npos) return json;
    
    return json.substr(pos + 1, end - pos - 1);
}

// Extrahiere Wert aus JSON String für ein bestimmtes Feld (String oder Zahl)
inline std::string ExtractJsonField(const std::string& jsonStr, const std::string& field)
{
    size_t pos = jsonStr.find("\"" + field + "\":");
    if (pos == std::string::npos) return "";
    
    pos += field.length() + 3; // Springe über ":
    
    // Überspringe Whitespace
    while (pos < jsonStr.length() && (jsonStr[pos] == ' ' || jsonStr[pos] == '\t')) {
        pos++;
    }
    
    // Check ob String (mit Anführungszeichen) oder Zahl
    if (jsonStr[pos] == '"') {
        // String-Wert
        pos++;
        size_t end = jsonStr.find("\"", pos);
        if (end == std::string::npos) return "";
        return jsonStr.substr(pos, end - pos);
    } else {
        // Numerischer Wert - lese bis zur nächsten Komma oder Klammer
        size_t end = pos;
        while (end < jsonStr.length() && jsonStr[end] != ',' && jsonStr[end] != '}' && jsonStr[end] != ']') {
            end++;
        }
        std::string numStr = jsonStr.substr(pos, end - pos);
        // Entferne Whitespace
        size_t lastNonSpace = numStr.find_last_not_of(" \t");
        if (lastNonSpace != std::string::npos) {
            return numStr.substr(0, lastNonSpace + 1);
        }
        return numStr;
    }
}

// Text Wrapping für lange Nachrichten
inline std::vector<std::string> WrapText(const std::string& text, size_t maxCharsPerLine = 60)
{
    std::vector<std::string> lines;
    std::string currentLine;
    std::istringstream stream(text);
    std::string word;
    
    while (stream >> word) {
        if ((currentLine + word).length() > maxCharsPerLine && !currentLine.empty()) {
            lines.push_back(currentLine);
            currentLine = word;
        } else {
            if (!currentLine.empty()) currentLine += " ";
            currentLine += word;
        }
    }
    
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    
    return lines;
}

void RunGui(Presentation::ViewModel::MainViewModel &vm)
{
#ifdef USE_SFML
    // Initialize Services (LoginService MUSS vor ApiService initialisiert werden!)
    Services::LoginService::Initialize();
    Services::ApiService::Initialize("127.0.0.1", 5000);
    
    // Check if user is already logged in
    bool userAlreadyLoggedIn = Services::LoginService::IsLoggedIn();

    sf::RenderWindow window(sf::VideoMode(1200, 700), "Text Extraction - MVVM (SFML)");
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        // try default
    }
    
    // Initialize UI
    auto sidebar = std::make_unique<UI::Sidebar>(font, 250.f, 700.f);
    
    // Add menu items to ribbons
    /*
    sidebar->addItemToRibbon("Home", {"New Project", nullptr});
    sidebar->addItemToRibbon("Home", {"Open", nullptr});
    sidebar->addItemToRibbon("Upload", {"Upload File", nullptr});
    sidebar->addItemToRibbon("Upload", {"Batch Upload", nullptr});
    sidebar->addItemToRibbon("Extraktion", {"Start Extraction", nullptr});
    sidebar->addItemToRibbon("Extraktion", {"View Results", nullptr});
    sidebar->addItemToRibbon("Admin Panel", {"Manage Users", nullptr});
    sidebar->addItemToRibbon("Admin Panel", {"System Settings", nullptr});
    sidebar->addItemToRibbon("Einstellungen", {"Preferences", nullptr});
    sidebar->addItemToRibbon("Einstellungen", {"About", nullptr});
    sidebar->addItemToRibbon("Profil", {"My Profile", nullptr});
    sidebar->addItemToRibbon("Profil", {"Logout", nullptr});
    */

    // API State
    std::string apiUrl = Services::ApiService::GetApiUrl();
    std::string apiResponse = "";
    int activeTab = 0; // 0 = Home, 1 = Upload, 4 = Einstellungen, 5 = Profil
    bool showApiUrlInput = false;
    std::string urlInput = apiUrl;
    
    // Upload State
    std::string selectedFilePath = "";
    std::string uploadStatus = "";
    double uploadProgress = 0.0; // 0.0 to 1.0
    bool isUploading = false;
    bool showUploadSuccess = false;
    bool uploadButtonPressed = false; // Verhindert mehrfache Uploads beim Halten des Buttons
    
    // Extraction State
    struct DocumentInfo {
        std::string fileId;
        std::string fileName;
        std::string uploadDate;
        std::string fileSize;
    };
    std::vector<DocumentInfo> documents;
    bool showExtractionDetail = false;
    DocumentInfo selectedDocument{"", "", "", ""};
    std::string extractedText = "";
    bool documentsLoaded = false;
    bool loadingDocuments = false;
    bool isExtracting = false;
    std::string extractionStatus = "";
    bool extractionCompleted = false;
    std::string extractionMethod = "";
    std::string completedAt = "";
    float textScrollOffset = 0.f;  // Für scrollbare Text-Box
    bool isDraggingScrollBar = false;
    float maxScrollOffset = 0.f;
    
    // Login State
    std::string loginUsername = "";
    std::string loginPassword = "";
    std::string loginError = "";
    bool isLoginInputMode = true; // false = show user data
    Services::LoginInfo userInfo{"Guest", "", "User", ""};
    bool showPasswordInput = false; // Show actual password chars or dots
    bool usernameFocused = false;
    bool passwordFocused = false;
    
    // Initialize with logged-in state if user exists
    if (userAlreadyLoggedIn) {
        isLoginInputMode = false;
        auto info = Services::LoginService::GetLoginInfo();
        if (info.has_value()) {
            userInfo = info.value();
}
    } else {
        std::cout << "Zeige Login-Formular" << std::endl;
    }
    
    // Setze Ribbon-Sichtbarkeit basierend auf Login-Status und Role
    auto updateRibbonVisibility = [&]() {
        bool isLoggedIn = Services::LoginService::IsLoggedIn();
        bool isAdmin = (userInfo.role == "Administrator");
        
        // 0=Home (immer sichtbar), 1=Upload, 2=Extraktion, 3=AdminPanel, 4=Einstellungen, 5=Profil
        sidebar->setRibbonVisible(0, true);      // Home - immer sichtbar
        sidebar->setRibbonVisible(1, isLoggedIn); // Upload - nur wenn angemeldet
        sidebar->setRibbonVisible(2, isLoggedIn); // Extraktion - nur wenn angemeldet
        sidebar->setRibbonVisible(3, isAdmin);    // AdminPanel - nur wenn Administrator
        sidebar->setRibbonVisible(4, true);       // Einstellungen - immer sichtbar
        sidebar->setRibbonVisible(5, true);       // Profil - immer sichtbar
    };
    updateRibbonVisibility(); // Initial setzen
    
    while (window.isOpen()) {
        float sidebarWidth = sidebar->getWidth();
        
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            
            // Pass events to UI
            sidebar->handleEvent(event);
            
            // Handle text input for URL settings
            if (showApiUrlInput && event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 8 && !urlInput.empty()) { // Backspace
                    urlInput.pop_back();
                } else if (event.text.unicode == 13) { // Enter
                    Services::ApiService::SetApiUrl(urlInput);
                    apiUrl = urlInput;
                    showApiUrlInput = false;
                } else if (event.text.unicode == 27) { // Escape
                    showApiUrlInput = false;
                    urlInput = apiUrl;
                } else if (event.text.unicode >= 32 && event.text.unicode < 127) {
                    urlInput += static_cast<char>(event.text.unicode);
                }
            }
            
            // Handle text input for login
            if (activeTab == 5 && isLoginInputMode && event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 9) { // Tab key - switch fields
                    usernameFocused = !usernameFocused;
                    passwordFocused = !passwordFocused;
                } else if (event.text.unicode == 8) { // Backspace
                    if (usernameFocused && !loginUsername.empty()) {
                        loginUsername.pop_back();
                    } else if (passwordFocused && !loginPassword.empty()) {
                        loginPassword.pop_back();
                    }
                } else if (event.text.unicode >= 32 && event.text.unicode < 127) {
                    if (usernameFocused) {
                        loginUsername += static_cast<char>(event.text.unicode);
                    } else if (passwordFocused) {
                        loginPassword += static_cast<char>(event.text.unicode);
                    }
                }
            }
            
            // Handle mouse clicks for login fields
            if (activeTab == 5 && isLoginInputMode && event.type == sf::Event::MouseButtonPressed) {
                // Check click on username field
                if (event.mouseButton.x >= sidebarWidth + 50.f && event.mouseButton.x <= sidebarWidth + 350.f &&
                    event.mouseButton.y >= 160.f && event.mouseButton.y <= 195.f) {
                    usernameFocused = true;
                    passwordFocused = false;
                } 
                // Check click on password field
                else if (event.mouseButton.x >= sidebarWidth + 50.f && event.mouseButton.x <= sidebarWidth + 350.f &&
                         event.mouseButton.y >= 240.f && event.mouseButton.y <= 275.f) {
                    usernameFocused = false;
                    passwordFocused = true;
                }
                // Check for login button click
                else if (event.mouseButton.x >= sidebarWidth + 50.f && event.mouseButton.x <= sidebarWidth + 150.f &&
                    event.mouseButton.y >= 300.f && event.mouseButton.y <= 335.f) {
                    // Perform login
                    Services::HttpResponse resp = Services::ApiService::Login(loginUsername, loginPassword);
                    if (resp.isSuccess) {
                        // Speichere das gehashte Passwort für Auth-Header
                        std::string hashedPassword = Services::ApiService::HashPassword(loginPassword);
                        std::cout << "Login erfolgreich. Speichere Credentials für Upload." << std::endl;
                        Services::LoginService::SaveLogin(loginUsername, resp.user_email, resp.user_role, hashedPassword);
                        // SetAuthCredentials mit Username und gehashtem Passwort
                        Services::ApiService::SetAuthCredentials(loginUsername, hashedPassword);
                        auto info = Services::LoginService::GetLoginInfo();
                        if (info.has_value()) {
                            userInfo = info.value();
                        }
                        isLoginInputMode = false;
                        loginError = "";
                        updateRibbonVisibility(); // Aktualisiere Sichtbarkeit nach Login
                    } else {
                        loginError = "Login failed: " + std::to_string(resp.statusCode);
                    }
                }
            }
            // Handle logout button click
            else if (activeTab == 5 && !isLoginInputMode && event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.x >= sidebarWidth + 50.f && event.mouseButton.x <= sidebarWidth + 170.f &&
                    event.mouseButton.y >= 280.f && event.mouseButton.y <= 315.f) {
                    Services::LoginService::ClearLogin();
                    isLoginInputMode = true;
                    loginUsername = "";
                    loginPassword = "";
                    loginError = "";
                    userInfo = {"Guest", "", "User", ""};
                    updateRibbonVisibility(); // Aktualisiere Sichtbarkeit nach Logout
                    activeTab = 0; // Gehe zurück zu Home
                }
            }
        }
        
        // Check if a ribbon was clicked to change tab
        int clickedRibbon = sidebar->getLastClickedRibbon();
        if (clickedRibbon >= 0) {
            // Map ribbons to tabs: Home=0, Upload=1, Extraktion=2, Admin=3, Einstellungen=4, Profil=5
            activeTab = clickedRibbon;
            sidebar->resetClickedRibbon();
        }
        
        window.clear(sf::Color::White);
        
        // Draw sidebar
        sidebar->draw(window);
        
        // Draw main content area
        sf::RectangleShape contentArea(sf::Vector2f(1200.f - sidebarWidth, 700.f));
        contentArea.setPosition(sidebarWidth, 0.f);
        contentArea.setFillColor(sf::Color(240, 240, 240));
        window.draw(contentArea);
        
        // Draw header text
        sf::Text header("Text Extraction System", font, 28u);
        header.setFillColor(sf::Color::Black);
        header.setPosition(sidebarWidth + 20.f, 20.f);
        window.draw(header);
        
        // Content based on active tab
        if (activeTab == 0) { // Home - Show API Response
            sf::Text apiLabel("API Status:", font, 16u);
            apiLabel.setFillColor(sf::Color::Black);
            apiLabel.setPosition(sidebarWidth + 20.f, 70.f);
            window.draw(apiLabel);
            
            // Button to fetch API response
            sf::RectangleShape fetchBtn(sf::Vector2f(150.f, 35.f));
            fetchBtn.setPosition(sidebarWidth + 20.f, 100.f);
            fetchBtn.setFillColor(sf::Color(70, 130, 180));
            window.draw(fetchBtn);
            
            sf::Text fetchText("Fetch API Status", font, 12u);
            fetchText.setFillColor(sf::Color::White);
            fetchText.setPosition(sidebarWidth + 30.f, 110.f);
            window.draw(fetchText);
            
            // Check for click on button
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.x >= sidebarWidth + 20.f && event.mouseButton.x <= sidebarWidth + 170.f &&
                    event.mouseButton.y >= 100.f && event.mouseButton.y <= 135.f) {
                    Services::HttpResponse resp = Services::ApiService::Get("");
                    apiResponse = resp.isSuccess ? resp.body : "ERROR: " + std::to_string(resp.statusCode);
                }
            }
            
            // Display response
            sf::Text responseLabel("Response:", font, 14u);
            responseLabel.setFillColor(sf::Color::Black);
            responseLabel.setPosition(sidebarWidth + 20.f, 150.f);
            window.draw(responseLabel);
            
            // Limit response display to 500 chars
            std::string displayResponse = apiResponse.length() > 500 ? apiResponse.substr(0, 500) + "..." : apiResponse;
            sf::Text responseText(displayResponse, font, 11u);
            responseText.setFillColor(sf::Color(50, 50, 50));
            responseText.setPosition(sidebarWidth + 20.f, 180.f);
            window.draw(responseText);
            
        } else if (activeTab == 1) { // Upload - File Upload
            sf::Text uploadTitle(ToSFMLString("Datei hochladen"), font, 20u);
            uploadTitle.setFillColor(sf::Color::Black);
            uploadTitle.setPosition(sidebarWidth + 20.f, 70.f);
            window.draw(uploadTitle);
            
            // Datei auswählen Button
            sf::RectangleShape selectBtn(sf::Vector2f(200.f, 40.f));
            selectBtn.setPosition(sidebarWidth + 20.f, 120.f);
            selectBtn.setFillColor(sf::Color(70, 130, 180));
            window.draw(selectBtn);
            
            sf::Text selectText(ToSFMLString("Datei auswählen"), font, 14u);
            selectText.setFillColor(sf::Color::White);
            selectText.setPosition(sidebarWidth + 40.f, 130.f);
            window.draw(selectText);
            
            // Dateiname anzeigen
            if (!selectedFilePath.empty()) {
                size_t lastSlash = selectedFilePath.find_last_of("/\\");
                std::string fileName = (lastSlash != std::string::npos) ? selectedFilePath.substr(lastSlash + 1) : selectedFilePath;
                
                sf::Text fileLabel(ToSFMLString("Gewählte Datei:"), font, 12u);
                fileLabel.setFillColor(sf::Color::Black);
                fileLabel.setPosition(sidebarWidth + 20.f, 180.f);
                window.draw(fileLabel);
                
                sf::Text fileNameText(fileName, font, 12u);
                fileNameText.setFillColor(sf::Color(100, 100, 100));
                fileNameText.setPosition(sidebarWidth + 150.f, 180.f);
                window.draw(fileNameText);
                
                // Upload Button
                sf::RectangleShape uploadBtn(sf::Vector2f(150.f, 40.f));
                uploadBtn.setPosition(sidebarWidth + 20.f, 220.f);
                uploadBtn.setFillColor(isUploading ? sf::Color(150, 150, 150) : sf::Color(50, 150, 50));
                window.draw(uploadBtn);
                
                sf::Text uploadBtnText(ToSFMLString(isUploading ? "Lädt..." : "Hochladen"), font, 14u);
                uploadBtnText.setFillColor(sf::Color::White);
                uploadBtnText.setPosition(sidebarWidth + 40.f, 230.f);
                window.draw(uploadBtnText);
                
                // Fortschrittsbalken
                if (isUploading || uploadProgress > 0.0) {
                    sf::Text progressLabel(ToSFMLString("Fortschritt:"), font, 12u);
                    progressLabel.setFillColor(sf::Color::Black);
                    progressLabel.setPosition(sidebarWidth + 20.f, 280.f);
                    window.draw(progressLabel);
                    
                    // Hintergrund für Fortschrittsbalken
                    sf::RectangleShape progressBg(sf::Vector2f(500.f, 30.f));
                    progressBg.setPosition(sidebarWidth + 20.f, 310.f);
                    progressBg.setFillColor(sf::Color(200, 200, 200));
                    window.draw(progressBg);
                    
                    // Fortschrittsbalken
                    float progressWidth = 500.f * uploadProgress;
                    sf::RectangleShape progressBar(sf::Vector2f(progressWidth, 30.f));
                    progressBar.setPosition(sidebarWidth + 20.f, 310.f);
                    progressBar.setFillColor(sf::Color(50, 150, 50));
                    window.draw(progressBar);
                    
                    // Prozenttext
                    std::string percentText = std::to_string(static_cast<int>(uploadProgress * 100)) + "%";
                    sf::Text percentDisplay(percentText, font, 12u);
                    percentDisplay.setFillColor(sf::Color::Black);
                    percentDisplay.setPosition(sidebarWidth + 250.f, 318.f);
                    window.draw(percentDisplay);
                }
                
                // Upload Status / Erfolg / Fehler - mit schöner Formatierung
                if (showUploadSuccess || !uploadStatus.empty()) {
                    // Extrahiere die Nachricht aus der Response
                    std::string displayMessage;
                    if (showUploadSuccess) {
                        displayMessage = ToSFMLString("Datei erfolgreich hochgeladen!");
                    } else {
                        displayMessage = ExtractMessageFromJSON(uploadStatus);
                    }
                    
                    // Wrapper für lange Texte
                    auto wrappedLines = WrapText(displayMessage, 65);
                    
                    // Berechne Box-Größe
                    float boxWidth = 550.f;
                    float lineHeight = 20.f;
                    float boxHeight = (wrappedLines.size() * lineHeight) + 30.f;
                    float boxX = sidebarWidth + 20.f;
                    float boxY = 360.f;
                    
                    // Zeichne Hintergrund-Box
                    sf::RectangleShape messageBox(sf::Vector2f(boxWidth, boxHeight));
                    messageBox.setPosition(boxX, boxY);
                    messageBox.setFillColor(showUploadSuccess ? sf::Color(240, 255, 240) : sf::Color(255, 240, 240));
                    messageBox.setOutlineColor(showUploadSuccess ? sf::Color(50, 150, 50) : sf::Color(200, 50, 50));
                    messageBox.setOutlineThickness(2.f);
                    window.draw(messageBox);
                    
                    // Zeichne Text zeilenweise
                    float textY = boxY + 10.f;
                    for (const auto& line : wrappedLines) {
                        sf::Text lineText(ToSFMLString(line), font, 12u);
                        lineText.setFillColor(showUploadSuccess ? sf::Color(50, 150, 50) : sf::Color(200, 50, 50));
                        lineText.setPosition(boxX + 15.f, textY);
                        window.draw(lineText);
                        textY += lineHeight;
                    }
                }
            }
            
            // Click Handler für "Datei auswählen" Button
            if (event.type == sf::Event::MouseButtonPressed && !isUploading) {
                if (event.mouseButton.x >= sidebarWidth + 20.f && event.mouseButton.x <= sidebarWidth + 220.f &&
                    event.mouseButton.y >= 120.f && event.mouseButton.y <= 160.f) {
                    // Öffne Datei-Dialog mit besserem Error-Handling
                    std::string command;
                    #ifdef _WIN32
                        // Windows: PowerShell Datei-Dialog
                        command = "powershell -Command \"[System.Reflection.Assembly]::LoadWithPartialName('System.windows.forms') | Out-Null; $f = New-Object System.Windows.Forms.OpenFileDialog; $f.ShowDialog() | Out-Null; Write-Host $f.FileName\" > /tmp/selected_file.txt 2>/dev/null";
                    #else
                        // Linux/macOS: zenity - stderr zu /dev/null um GTK Warnings auszufiltern
                        command = "zenity --file-selection --title='Datei auswählen' > /tmp/selected_file.txt 2>/dev/null";
                    #endif
                    
                    int ret = system(command.c_str());
                    std::cout << "File dialog exit code: " << ret << std::endl;
                    
                    // Kleine Verzögerung um sicherzustellen dass die Datei geschrieben ist
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    
                    // Lese die ausgewählte Datei
                    std::ifstream file("/tmp/selected_file.txt");
                    if (file.is_open()) {
                        std::getline(file, selectedFilePath);
                        file.close();
                        
                        // Entferne alle Whitespace am Ende
                        while (!selectedFilePath.empty() && 
                               (selectedFilePath.back() == '\n' || 
                                selectedFilePath.back() == '\r' || 
                                selectedFilePath.back() == ' ')) {
                            selectedFilePath.pop_back();
                        }
                        
                        std::cout << "DEBUG: selectedFilePath nach Bereinigung: [" << selectedFilePath << "]" << std::endl;
                        
                        if (!selectedFilePath.empty()) {
                            std::cout << "Datei ausgewählt: " << selectedFilePath << std::endl;
                        } else {
                            std::cout << "Keine Datei ausgewählt (leer)" << std::endl;
                        }
                    } else {
                        std::cout << "Fehler beim Lesen von /tmp/selected_file.txt" << std::endl;
                    }
                }
            }
            
            // Click Handler für "Hochladen" Button
            if (event.type == sf::Event::MouseButtonPressed && !selectedFilePath.empty() && !isUploading && !uploadButtonPressed) {
                if (event.mouseButton.x >= sidebarWidth + 20.f && event.mouseButton.x <= sidebarWidth + 170.f &&
                    event.mouseButton.y >= 220.f && event.mouseButton.y <= 260.f) {
                    uploadButtonPressed = true; // Markiere Button als gedrückt
                    std::cout << "Upload Button geklickt. selectedFilePath: [" << selectedFilePath << "]" << std::endl;
                    
                    isUploading = true;
                    showUploadSuccess = false;
                    uploadStatus = "";
                    uploadProgress = 0.0;
                    
                    if (selectedFilePath.empty()) {
                        uploadStatus = "Fehler: Keine Datei ausgewählt";
                        isUploading = false;
                        std::cout << "Upload abgebrochen: selectedFilePath ist leer" << std::endl;
                    } else {
                        // Upload mit Progress-Callback
                        Services::HttpResponse resp = Services::ApiService::UploadFile(selectedFilePath,
                            [&uploadProgress](double progress) {
                                uploadProgress = progress;
                            });
                        
                        isUploading = false;
                        if (resp.isSuccess) {
                            showUploadSuccess = true;
                            uploadStatus = "";
                            std::cout << "Upload erfolgreich!" << std::endl;
                        } else {
                            showUploadSuccess = false;
                            uploadStatus = "Fehler: " + resp.body;
                            std::cout << "Upload fehlgeschlagen: " << resp.body << std::endl;
                        }
                    }
                }
            }
            
            // Reset uploadButtonPressed wenn Button losgelassen wird
            if (event.type == sf::Event::MouseButtonReleased) {
                uploadButtonPressed = false;
            }
            
        } else if (activeTab == 2) { // Extraktion - Document List & Extraction
            if (!showExtractionDetail) {
                // === DOCUMENT LIST VIEW ===
                sf::Text extractionTitle(ToSFMLString("Hochgeladene Dokumente"), font, 20u);
                extractionTitle.setFillColor(sf::Color::Black);
                extractionTitle.setPosition(sidebarWidth + 20.f, 70.f);
                window.draw(extractionTitle);
                
                // Lade Dokumente beim ersten Mal
                if (!documentsLoaded && !loadingDocuments) {
                    loadingDocuments = true;
                    // GET /api/Upload/my-documents/
                    Services::HttpResponse resp = Services::ApiService::Get("Upload/my-documents");
                    //std::cout << "Extraction API Response Status: " << resp.statusCode << std::endl;
                    //std::cout << "Extraction API Response Body: " << resp.body << std::endl;
                    
                    if (resp.isSuccess && !resp.body.empty()) {
                        documents.clear();
                        // Parse JSON Array - vereinfacht für [{fileId, fileName, uploadDate, fileSize}, ...]
                        size_t pos = 0;
                        int docCount = 0;
                        while ((pos = resp.body.find("{", pos)) != std::string::npos) {
                            size_t endPos = resp.body.find("}", pos);
                            if (endPos == std::string::npos) break;
                            
                            std::string objStr = resp.body.substr(pos, endPos - pos + 1);
                            
                            DocumentInfo doc;
                            doc.fileId = ExtractJsonField(objStr, "id");
                            doc.fileName = ExtractJsonField(objStr, "fileName");
                            doc.uploadDate = ExtractJsonField(objStr, "uploadedAt");
                            doc.fileSize = ExtractJsonField(objStr, "fileSize");
                            
                            if (!doc.fileId.empty()) {
                                documents.push_back(doc);
                                docCount++;
                                std::cout << "Dokument " << docCount << " geladen: " << doc.fileName << " (ID: " << doc.fileId << ")" << std::endl;
                            }
                            
                            pos = endPos + 1;
                        }
                        std::cout << "Total Dokumente geladen: " << docCount << std::endl;
                    } else {
                        std::cout << "Fehler beim Laden der Dokumente. Status: " << resp.statusCode << std::endl;
                    }
                    documentsLoaded = true;
                    loadingDocuments = false;
                }
                
                // Zeichne Dokumentenliste
                float docY = 120.f;
                for (size_t i = 0; i < documents.size(); ++i) {
                    const auto& doc = documents[i];
                    
                    // Document Item Box (größer für Button)
                    sf::RectangleShape docBox(sf::Vector2f(900.f, 70.f));
                    docBox.setPosition(sidebarWidth + 20.f, docY);
                    docBox.setFillColor(sf::Color(240, 240, 240));
                    docBox.setOutlineColor(sf::Color(180, 180, 180));
                    docBox.setOutlineThickness(1.f);
                    window.draw(docBox);
                    
                    // Dateiname (größer machen) - mit Truncation
                    std::string displayName = doc.fileName;
                    if (displayName.length() > 50) {
                        displayName = displayName.substr(0, 47) + "...";
                    }
                    sf::Text docNameText(ToSFMLString(displayName), font, 14u);
                    docNameText.setFillColor(sf::Color::Black);
                    docNameText.setPosition(sidebarWidth + 30.f, docY + 8.f);
                    window.draw(docNameText);
                    
                    // Datum und Größe - mit Truncation
                    std::string metaDisplay = "Upload: ";
                    std::string dateOnly = doc.uploadDate.length() > 19 ? 
                        doc.uploadDate.substr(0, 19) : doc.uploadDate;
                    metaDisplay += dateOnly + " | Größe: " + doc.fileSize + " B";
                    if (metaDisplay.length() > 80) {
                        metaDisplay = metaDisplay.substr(0, 77) + "...";
                    }
                    sf::Text docMetaText(ToSFMLString(metaDisplay), font, 11u);
                    docMetaText.setFillColor(sf::Color(100, 100, 100));
                    docMetaText.setPosition(sidebarWidth + 30.f, docY + 28.f);
                    window.draw(docMetaText);
                    
                    // Extract Button (rechts aligned, innerhalb der Box)
                    sf::RectangleShape extractBtn(sf::Vector2f(110.f, 40.f));
                    extractBtn.setPosition(sidebarWidth + 800.f, docY + 15.f);
                    extractBtn.setFillColor(sf::Color(70, 130, 180));
                    extractBtn.setOutlineColor(sf::Color(50, 100, 150));
                    extractBtn.setOutlineThickness(1.f);
                    window.draw(extractBtn);
                    
                    sf::Text extractBtnText(ToSFMLString("Öffnen"), font, 12u);
                    extractBtnText.setFillColor(sf::Color::White);
                    extractBtnText.setPosition(sidebarWidth + 823.f, docY + 20.f);
                    window.draw(extractBtnText);
                    
                    // Click Handler für Extract Button
                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (event.mouseButton.x >= sidebarWidth + 800.f && event.mouseButton.x <= sidebarWidth + 910.f &&
                            event.mouseButton.y >= docY + 15.f && event.mouseButton.y <= docY + 55.f) {
                            selectedDocument = doc;
                            extractedText = "";
                            extractionStatus = "";
                            extractionCompleted = false;
                            extractionMethod = "";
                            completedAt = "";
                            textScrollOffset = 0.f;
                            showExtractionDetail = true;
                            
                            // Lade vorhandene Extraktion vom Server
                            std::string extractionUrl = "Extraction/result/" + doc.fileId;
                            Services::HttpResponse resp = Services::ApiService::Get(extractionUrl);
                            
                            if (resp.isSuccess && !resp.body.empty()) {
                                // Parse JSON Response
                                extractedText = ExtractJsonField(resp.body, "extractedText");
                                extractionMethod = ExtractJsonField(resp.body, "extractionMethod");
                                completedAt = ExtractJsonField(resp.body, "completedAt");
                                
                                if (!extractedText.empty()) {
                                    extractionStatus = "Vorhandene Extraktion geladen";
                                    extractionCompleted = true;
                                    std::cout << "Vorhandene Extraktion geladen für: " << doc.fileName << std::endl;
                                    std::cout << "ExtractionMethod: " << extractionMethod << std::endl;
                                    std::cout << "CompletedAt: " << completedAt << std::endl;
                                } else {
                                    extractionStatus = "Keine Extraktion vorhanden";
                                    extractionCompleted = false;
                                }
                            } else {
                                extractedText = "";
                                extractionStatus = "";
                                extractionCompleted = false;
                                std::cout << "Keine vorhandene Extraktion für: " << doc.fileName << " (Status: " << resp.statusCode << ")" << std::endl;
                            }
                            
                            std::cout << "Extraction Detail für: " << doc.fileName << " (ID: " << doc.fileId << ")" << std::endl;
                        }
                    }
                    
                    docY += 85.f;
                }
                
                // "Keine Dokumente" Nachricht
                if (documents.empty() && documentsLoaded) {
                    sf::Text noDocsText(ToSFMLString("Keine hochgeladenen Dokumente vorhanden"), font, 14u);
                    noDocsText.setFillColor(sf::Color(150, 150, 150));
                    noDocsText.setPosition(sidebarWidth + 20.f, 150.f);
                    window.draw(noDocsText);
                }
            } else {
                // === EXTRACTION DETAIL VIEW ===
                sf::Text detailTitle(ToSFMLString("Extraktion: " + selectedDocument.fileName), font, 18u);
                detailTitle.setFillColor(sf::Color::Black);
                detailTitle.setPosition(sidebarWidth + 20.f, 70.f);
                window.draw(detailTitle);
                
                // Zurück Button
                sf::RectangleShape backBtn(sf::Vector2f(100.f, 35.f));
                backBtn.setPosition(sidebarWidth + 20.f, 110.f);
                backBtn.setFillColor(sf::Color(100, 100, 100));
                window.draw(backBtn);
                
                sf::Text backBtnText(ToSFMLString("< Zurück"), font, 12u);
                backBtnText.setFillColor(sf::Color::White);
                backBtnText.setPosition(sidebarWidth + 35.f, 118.f);
                window.draw(backBtnText);
                
                // Click Handler für Zurück Button
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.x >= sidebarWidth + 20.f && event.mouseButton.x <= sidebarWidth + 120.f &&
                        event.mouseButton.y >= 110.f && event.mouseButton.y <= 145.f) {
                        showExtractionDetail = false;
                        std::cout << "Zurück zur Dokumentenliste" << std::endl;
                    }
                }
                
                // Extraction Button
                sf::RectangleShape extractBtn(sf::Vector2f(150.f, 35.f));
                extractBtn.setPosition(sidebarWidth + 130.f, 110.f);
                extractBtn.setFillColor(isExtracting ? sf::Color(100, 100, 100) : sf::Color(34, 139, 34));
                extractBtn.setOutlineColor(sf::Color(20, 100, 20));
                extractBtn.setOutlineThickness(1.f);
                window.draw(extractBtn);
                
                sf::Text extractBtnText(ToSFMLString(isExtracting ? "Extrahiert..." : "Extraktion starten"), font, 12u);
                extractBtnText.setFillColor(sf::Color::White);
                extractBtnText.setPosition(sidebarWidth + 145.f, 118.f);
                window.draw(extractBtnText);
                
                // Click Handler für Extract Button
                if (event.type == sf::Event::MouseButtonPressed && !isExtracting) {
                    if (event.mouseButton.x >= sidebarWidth + 130.f && event.mouseButton.x <= sidebarWidth + 280.f &&
                        event.mouseButton.y >= 110.f && event.mouseButton.y <= 145.f) {
                        isExtracting = true;
                        extractionStatus = "Starte Extraktion...";
                        extractionCompleted = false;
                        
                        // POST Request: /api/Extraction/{documentId}
                        std::string extractionUrl = "Extraction/" + selectedDocument.fileId;
                        std::string jsonBody = R"({
                            "enableOCR": false,
                            "language": "de",
                            "maxPages": 5,
                            "preserveFormatting": false,
                            "enableLanguageModel": false,
                            "maxSummaryLength": 0
                        })";
                        
                        Services::HttpResponse resp = Services::ApiService::Post(extractionUrl, jsonBody);
                        
                        if (resp.isSuccess) {
                            extractionStatus = "Extraktion erfolgreich!";
                            extractedText = resp.body;
                            extractionCompleted = true;
                            std::cout << "Extraction erfolgreich für: " << selectedDocument.fileName << std::endl;
                            std::cout << "Response: " << resp.body << std::endl;
                        } else {
                            extractionStatus = "Fehler bei Extraktion: Status " + std::to_string(resp.statusCode);
                            extractedText = "Fehler beim Extrahieren des Textes.";
                            std::cout << "Extraction fehlgeschlagen. Status: " << resp.statusCode << std::endl;
                            std::cout << "Response: " << resp.body << std::endl;
                        }
                        
                        isExtracting = false;
                    }
                }
                
                // Document Info Panel
                sf::RectangleShape infoPanel(sf::Vector2f(900.f, 70.f));
                infoPanel.setPosition(sidebarWidth + 20.f, 160.f);
                infoPanel.setFillColor(sf::Color(235, 245, 255));
                infoPanel.setOutlineColor(sf::Color(100, 150, 200));
                infoPanel.setOutlineThickness(1.f);
                window.draw(infoPanel);
                
                // Document Info - Filename
                sf::Text docNameLabel(ToSFMLString("Datei: "), font, 11u);
                docNameLabel.setFillColor(sf::Color(50, 50, 50));
                docNameLabel.setPosition(sidebarWidth + 30.f, 170.f);
                window.draw(docNameLabel);
                
                sf::Text docNameValue(ToSFMLString(selectedDocument.fileName), font, 11u);
                docNameValue.setFillColor(sf::Color::Black);
                docNameValue.setPosition(sidebarWidth + 90.f, 170.f);
                window.draw(docNameValue);
                
                // Document Info - Upload Date (truncated)
                sf::Text docDateLabel(ToSFMLString("Upload: "), font, 11u);
                docDateLabel.setFillColor(sf::Color(50, 50, 50));
                docDateLabel.setPosition(sidebarWidth + 450.f, 170.f);
                window.draw(docDateLabel);
                
                std::string displayUploadDate = selectedDocument.uploadDate.length() > 19 ? 
                    selectedDocument.uploadDate.substr(0, 19) : selectedDocument.uploadDate;
                sf::Text docDateValue(ToSFMLString(displayUploadDate), font, 11u);
                docDateValue.setFillColor(sf::Color::Black);
                docDateValue.setPosition(sidebarWidth + 530.f, 170.f);
                window.draw(docDateValue);
                
                // Document Info - File Size
                sf::Text docSizeLabel(ToSFMLString("Größe: "), font, 11u);
                docSizeLabel.setFillColor(sf::Color(50, 50, 50));
                docSizeLabel.setPosition(sidebarWidth + 30.f, 190.f);
                window.draw(docSizeLabel);
                
                sf::Text docSizeValue(ToSFMLString(selectedDocument.fileSize + " Bytes"), font, 11u);
                docSizeValue.setFillColor(sf::Color::Black);
                docSizeValue.setPosition(sidebarWidth + 110.f, 190.f);
                window.draw(docSizeValue);
                
                // Extracted Text Box Header
                sf::Text textBoxHeader(ToSFMLString("Extrahierter Text:"), font, 13u);
                textBoxHeader.setFillColor(sf::Color::Black);
                textBoxHeader.setPosition(sidebarWidth + 20.f, 250.f);
                window.draw(textBoxHeader);
                
                // Metadata Panel (ExtractionMethod & CompletedAt)
                if (extractionCompleted && !extractionMethod.empty()) {
                    sf::RectangleShape metadataPanel(sf::Vector2f(900.f, 25.f));
                    metadataPanel.setPosition(sidebarWidth + 20.f, 275.f);
                    metadataPanel.setFillColor(sf::Color(245, 245, 245));
                    metadataPanel.setOutlineColor(sf::Color(200, 200, 200));
                    metadataPanel.setOutlineThickness(1.f);
                    window.draw(metadataPanel);
                    
                    sf::Text methodLabel(ToSFMLString("Methode: "), font, 10u);
                    methodLabel.setFillColor(sf::Color(80, 80, 80));
                    methodLabel.setPosition(sidebarWidth + 30.f, 280.f);
                    window.draw(methodLabel);
                    
                    std::string displayMethod = extractionMethod;
                    if (displayMethod.length() > 30) {
                        displayMethod = displayMethod.substr(0, 27) + "...";
                    }
                    sf::Text methodValue(ToSFMLString(displayMethod), font, 10u);
                    methodValue.setFillColor(sf::Color::Black);
                    methodValue.setPosition(sidebarWidth + 110.f, 280.f);
                    window.draw(methodValue);
                    
                    if (!completedAt.empty()) {
                        sf::Text dateLabel(ToSFMLString(" | Abgeschlossen: "), font, 10u);
                        dateLabel.setFillColor(sf::Color(80, 80, 80));
                        dateLabel.setPosition(sidebarWidth + 350.f, 280.f);
                        window.draw(dateLabel);
                        
                        std::string displayCompletedAt = completedAt.length() > 19 ? 
                            completedAt.substr(0, 19) : completedAt;
                        sf::Text dateValue(ToSFMLString(displayCompletedAt), font, 10u);
                        dateValue.setFillColor(sf::Color::Black);
                        dateValue.setPosition(sidebarWidth + 540.f, 280.f);
                        window.draw(dateValue);
                    }
                }
                
                // Text Area für extraierten Text (scrollbar)
                float textBoxY = extractionCompleted && !extractionMethod.empty() ? 310.f : 280.f;
                float textBoxHeight = extractionCompleted && !extractionMethod.empty() ? 310.f : 330.f;
                
                sf::RectangleShape textBox(sf::Vector2f(900.f, textBoxHeight));
                textBox.setPosition(sidebarWidth + 20.f, textBoxY);
                textBox.setFillColor(sf::Color(255, 255, 255));
                textBox.setOutlineColor(sf::Color(180, 180, 180));
                textBox.setOutlineThickness(1.f);
                window.draw(textBox);
                
                // Scroll-Handling mit Mausrad (weniger empfindlich)
                if (event.type == sf::Event::MouseWheelScrolled && extractionCompleted && !extractedText.empty()) {
                    // Prüfe ob Maus über Text-Box ist
                    if (event.mouseWheelScroll.x >= sidebarWidth + 20.f && event.mouseWheelScroll.x <= sidebarWidth + 920.f &&
                        event.mouseWheelScroll.y >= textBoxY && event.mouseWheelScroll.y <= textBoxY + textBoxHeight) {
                        textScrollOffset -= event.mouseWheelScroll.delta * 10.f;  // Reduziert von 30.f auf 10.f
                        if (textScrollOffset < 0.f) textScrollOffset = 0.f;
                        if (textScrollOffset > maxScrollOffset) textScrollOffset = maxScrollOffset;
                    }
                }
                
                // Scroll-Bar Mouse Input (Press & Drag)
                if (event.type == sf::Event::MouseButtonPressed && extractionCompleted && !extractedText.empty()) {
                    auto textLines = WrapText(extractedText, 110);
                    if ((int)textLines.size() > (int)(textBoxHeight / 18.f) - 1) {
                        float totalHeight = textLines.size() * 18.f;
                        float scrollRatio = textBoxHeight / totalHeight;
                        float scrollBarHeight = textBoxHeight * scrollRatio;
                        float scrollBarY = textBoxY + (textScrollOffset / totalHeight) * textBoxHeight;
                        
                        // Check ob auf Scrollbar geklickt
                        if (event.mouseButton.x >= sidebarWidth + 910.f && event.mouseButton.x <= sidebarWidth + 918.f &&
                            event.mouseButton.y >= scrollBarY && event.mouseButton.y <= scrollBarY + scrollBarHeight) {
                            isDraggingScrollBar = true;
                            maxScrollOffset = totalHeight - textBoxHeight;
                        }
                    }
                }
                
                // Scroll-Bar Release
                if (event.type == sf::Event::MouseButtonReleased) {
                    isDraggingScrollBar = false;
                }
                
                // Scroll-Bar Dragging (im Main Loop)
                if (isDraggingScrollBar && extractionCompleted && !extractedText.empty()) {
                    auto textLines = WrapText(extractedText, 110);
                    float totalHeight = textLines.size() * 18.f;
                    
                    // Berechne neue Scroll-Position basierend auf Maus-Position
                    float mouseY = sf::Mouse::getPosition(window).y;
                    float relativeY = mouseY - textBoxY;
                    if (relativeY < 0.f) relativeY = 0.f;
                    if (relativeY > textBoxHeight) relativeY = textBoxHeight;
                    
                    textScrollOffset = (relativeY / textBoxHeight) * totalHeight;
                    if (textScrollOffset < 0.f) textScrollOffset = 0.f;
                    maxScrollOffset = totalHeight - textBoxHeight;
                    if (textScrollOffset > maxScrollOffset) textScrollOffset = maxScrollOffset;
                }
                
                // Status Text (wenn noch nicht extrahiert)
                if (!extractionCompleted && extractionStatus.empty()) {
                    sf::Text placeholderText(ToSFMLString("Klicke auf 'Extraktion starten' um den Text zu extrahieren"), font, 12u);
                    placeholderText.setFillColor(sf::Color(150, 150, 150));
                    placeholderText.setPosition(sidebarWidth + 30.f, textBoxY + 20.f);
                    window.draw(placeholderText);
                } else if (extractionCompleted && !extractedText.empty()) {
                    // Zeige extrahierten Text mit Scrolling
                    auto textLines = WrapText(extractedText, 110);
                    float textY = textBoxY + 10.f - textScrollOffset;
                    int lineCount = 0;
                    int maxVisibleLines = (int)(textBoxHeight / 18.f) - 1;
                    
                    for (size_t i = 0; i < textLines.size(); ++i) {
                        float adjustedY = textY + (i * 18.f);
                        
                        // Nur zeichnen wenn vertikal sichtbar
                        if (adjustedY >= textBoxY - 20.f && adjustedY <= textBoxY + textBoxHeight) {
                            // Begrenze Textlänge um sicherzustellen dass nichts hinausragt
                            std::string displayLine = textLines[i];
                            if (displayLine.length() > 110) {
                                displayLine = displayLine.substr(0, 107) + "...";
                            }
                            
                            sf::Text lineText(ToSFMLString(displayLine), font, 11u);
                            lineText.setFillColor(sf::Color::Black);
                            lineText.setPosition(sidebarWidth + 30.f, adjustedY);
                            
                            // Nur zeichnen wenn innerhalb der Box-Grenzen
                            if (adjustedY >= textBoxY && adjustedY <= textBoxY + textBoxHeight - 5.f) {
                                window.draw(lineText);
                            }
                            lineCount++;
                        }
                    }
                    
                    // Scrollbar-Indikator zeichnen wenn nötig
                    if ((int)textLines.size() > maxVisibleLines) {
                        float totalHeight = textLines.size() * 18.f;
                        float scrollRatio = textBoxHeight / totalHeight;
                        float scrollBarHeight = textBoxHeight * scrollRatio;
                        float scrollBarY = textBoxY + (textScrollOffset / totalHeight) * textBoxHeight;
                        
                        sf::RectangleShape scrollBar(sf::Vector2f(8.f, scrollBarHeight));
                        scrollBar.setPosition(sidebarWidth + 910.f, scrollBarY);
                        scrollBar.setFillColor(isDraggingScrollBar ? sf::Color(100, 100, 200) : sf::Color(150, 150, 150));
                        scrollBar.setOutlineColor(sf::Color(100, 100, 100));
                        scrollBar.setOutlineThickness(1.f);
                        window.draw(scrollBar);
                    }
                    
                    // Text Statistics
                    int charCount = extractedText.length();
                    int wordCount = 0;
                    bool inWord = false;
                    for (char c : extractedText) {
                        if (std::isspace(c)) {
                            inWord = false;
                        } else if (!inWord) {
                            wordCount++;
                            inWord = true;
                        }
                    }
                    
                    sf::RectangleShape statsPanel(sf::Vector2f(900.f, 40.f));
                    statsPanel.setPosition(sidebarWidth + 20.f, textBoxY + textBoxHeight + 5.f);
                    statsPanel.setFillColor(sf::Color(240, 250, 240));
                    statsPanel.setOutlineColor(sf::Color(150, 200, 150));
                    statsPanel.setOutlineThickness(1.f);
                    window.draw(statsPanel);
                    
                    std::string statsText = "Zeichen: " + std::to_string(charCount) + " | Wörter: " + std::to_string(wordCount) + " | Zeilen: " + std::to_string(textLines.size());
                    sf::Text statsLabel(ToSFMLString(statsText), font, 11u);
                    statsLabel.setFillColor(sf::Color(50, 100, 50));
                    statsLabel.setPosition(sidebarWidth + 30.f, textBoxY + textBoxHeight + 10.f);
                    window.draw(statsLabel);
                }
            }
            
        } else if (activeTab == 4) { // Einstellungen - API URL Settings
            sf::Text settingsTitle("API Configuration", font, 20u);
            settingsTitle.setFillColor(sf::Color::Black);
            settingsTitle.setPosition(sidebarWidth + 20.f, 70.f);
            window.draw(settingsTitle);
            
            sf::Text urlLabel("API Base URL:", font, 14u);
            urlLabel.setFillColor(sf::Color::Black);
            urlLabel.setPosition(sidebarWidth + 20.f, 120.f);
            window.draw(urlLabel);
            
            // URL input box
            sf::RectangleShape urlBox(sf::Vector2f(400.f, 35.f));
            urlBox.setPosition(sidebarWidth + 20.f, 150.f);
            urlBox.setFillColor(showApiUrlInput ? sf::Color(255, 255, 200) : sf::Color::White);
            urlBox.setOutlineColor(sf::Color::Black);
            urlBox.setOutlineThickness(2.f);
            window.draw(urlBox);
            
            std::string displayUrl = showApiUrlInput ? urlInput : apiUrl;
            sf::Text urlDisplay(displayUrl, font, 12u);
            urlDisplay.setFillColor(sf::Color::Black);
            urlDisplay.setPosition(sidebarWidth + 30.f, 160.f);
            window.draw(urlDisplay);
            
            // Edit button
            sf::RectangleShape editBtn(sf::Vector2f(100.f, 35.f));
            editBtn.setPosition(sidebarWidth + 440.f, 150.f);
            editBtn.setFillColor(sf::Color(100, 150, 100));
            window.draw(editBtn);
            
            sf::Text editText("Edit", font, 12u);
            editText.setFillColor(sf::Color::White);
            editText.setPosition(sidebarWidth + 460.f, 160.f);
            window.draw(editText);
            
            // Check for click on edit button
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.x >= sidebarWidth + 440.f && event.mouseButton.x <= sidebarWidth + 540.f &&
                    event.mouseButton.y >= 150.f && event.mouseButton.y <= 185.f) {
                    showApiUrlInput = !showApiUrlInput;
                    urlInput = apiUrl;
                }
            }
            
            sf::Text hint("(Press Enter to save, Escape to cancel)", font, 10u);
            hint.setFillColor(sf::Color(150, 150, 150));
            hint.setPosition(sidebarWidth + 20.f, 195.f);
            window.draw(hint);
            
            // Test connection button
            sf::RectangleShape testBtn(sf::Vector2f(200.f, 35.f));
            testBtn.setPosition(sidebarWidth + 20.f, 250.f);
            testBtn.setFillColor(sf::Color(70, 130, 180));
            window.draw(testBtn);
            
            sf::Text testText("Test Connection", font, 12u);
            testText.setFillColor(sf::Color::White);
            testText.setPosition(sidebarWidth + 40.f, 260.f);
            window.draw(testText);
            
            // Check for click on test button
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.x >= sidebarWidth + 20.f && event.mouseButton.x <= sidebarWidth + 220.f &&
                    event.mouseButton.y >= 250.f && event.mouseButton.y <= 285.f) {
                    bool connected = Services::ApiService::CheckConnection();
                    apiResponse = connected ? "Connected successfully!" : "Connection failed!";
                }
            }
            
            if (!apiResponse.empty()) {
                sf::Text connStatus(apiResponse, font, 12u);
                connStatus.setFillColor(apiResponse.find("") != std::string::npos ? sf::Color(50, 150, 50) : sf::Color(200, 50, 50));
                connStatus.setPosition(sidebarWidth + 20.f, 300.f);
                window.draw(connStatus);
            }
        } else if (activeTab == 5) { // Profil - Login/User Info
            if (isLoginInputMode) {
                // LOGIN FORM
                sf::Text loginTitle("Login", font, 24u);
                loginTitle.setFillColor(sf::Color::Black);
                loginTitle.setPosition(sidebarWidth + 50.f, 70.f);
                window.draw(loginTitle);
                
                // Username label and input
                sf::Text userLabel("Username:", font, 14u);
                userLabel.setFillColor(sf::Color::Black);
                userLabel.setPosition(sidebarWidth + 50.f, 130.f);
                window.draw(userLabel);
                
                sf::RectangleShape userBox(sf::Vector2f(300.f, 35.f));
                userBox.setPosition(sidebarWidth + 50.f, 160.f);
                userBox.setFillColor(sf::Color::White);
                userBox.setOutlineColor(usernameFocused ? sf::Color(70, 130, 180) : sf::Color::Black);
                userBox.setOutlineThickness(usernameFocused ? 2.f : 1.f);
                window.draw(userBox);
                
                sf::Text userDisplay(loginUsername, font, 12u);
                userDisplay.setFillColor(sf::Color::Black);
                userDisplay.setPosition(sidebarWidth + 60.f, 170.f);
                window.draw(userDisplay);
                
                // Password label and input
                sf::Text passLabel("Password:", font, 14u);
                passLabel.setFillColor(sf::Color::Black);
                passLabel.setPosition(sidebarWidth + 50.f, 210.f);
                window.draw(passLabel);
                
                sf::RectangleShape passBox(sf::Vector2f(300.f, 35.f));
                passBox.setPosition(sidebarWidth + 50.f, 240.f);
                passBox.setFillColor(sf::Color::White);
                passBox.setOutlineColor(passwordFocused ? sf::Color(70, 130, 180) : sf::Color::Black);
                passBox.setOutlineThickness(passwordFocused ? 2.f : 1.f);
                window.draw(passBox);
                
                // Show dots for password or actual text
                std::string displayPassword(loginPassword.length(), '*');
                sf::Text passDisplay(displayPassword, font, 12u);
                passDisplay.setFillColor(sf::Color::Black);
                passDisplay.setPosition(sidebarWidth + 60.f, 250.f);
                window.draw(passDisplay);
                
                // Login button
                sf::RectangleShape loginBtn(sf::Vector2f(100.f, 35.f));
                loginBtn.setPosition(sidebarWidth + 50.f, 300.f);
                loginBtn.setFillColor(sf::Color(70, 130, 180));
                window.draw(loginBtn);
                
                sf::Text loginBtnText("Login", font, 12u);
                loginBtnText.setFillColor(sf::Color::White);
                loginBtnText.setPosition(sidebarWidth + 75.f, 310.f);
                window.draw(loginBtnText);
                
                // Show error message
                if (!loginError.empty()) {
                    sf::Text errorMsg(loginError, font, 11u);
                    errorMsg.setFillColor(sf::Color(200, 50, 50));
                    errorMsg.setPosition(sidebarWidth + 50.f, 350.f);
                    window.draw(errorMsg);
                }
                
            } else {
                // USER INFO DISPLAY
                sf::Text profileTitle("User Profile", font, 24u);
                profileTitle.setFillColor(sf::Color::Black);
                profileTitle.setPosition(sidebarWidth + 50.f, 70.f);
                window.draw(profileTitle);
                
                sf::Text userInfoLabel("Username:", font, 14u);
                userInfoLabel.setFillColor(sf::Color(100, 100, 100));
                userInfoLabel.setPosition(sidebarWidth + 50.f, 130.f);
                window.draw(userInfoLabel);
                
                sf::Text userInfoValue(userInfo.username, font, 13u);
                userInfoValue.setFillColor(sf::Color::Black);
                userInfoValue.setPosition(sidebarWidth + 150.f, 130.f);
                window.draw(userInfoValue);
                
                sf::Text emailLabel("Email:", font, 14u);
                emailLabel.setFillColor(sf::Color(100, 100, 100));
                emailLabel.setPosition(sidebarWidth + 50.f, 170.f);
                window.draw(emailLabel);
                
                sf::Text emailValue(userInfo.email.empty() ? "Not set" : userInfo.email, font, 13u);
                emailValue.setFillColor(sf::Color::Black);
                emailValue.setPosition(sidebarWidth + 150.f, 170.f);
                window.draw(emailValue);
                
                sf::Text roleLabel("Role:", font, 14u);
                roleLabel.setFillColor(sf::Color(100, 100, 100));
                roleLabel.setPosition(sidebarWidth + 50.f, 210.f);
                window.draw(roleLabel);
                
                sf::Text roleValue(userInfo.role.empty() ? "User" : userInfo.role, font, 13u);
                roleValue.setFillColor(sf::Color::Black);
                roleValue.setPosition(sidebarWidth + 150.f, 210.f);
                window.draw(roleValue);
                
                // Logout button
                sf::RectangleShape logoutBtn(sf::Vector2f(120.f, 35.f));
                logoutBtn.setPosition(sidebarWidth + 50.f, 280.f);
                logoutBtn.setFillColor(sf::Color(200, 50, 50));
                window.draw(logoutBtn);
                
                sf::Text logoutBtnText("Logout", font, 12u);
                logoutBtnText.setFillColor(sf::Color::White);
                logoutBtnText.setPosition(sidebarWidth + 68.f, 290.f);
                window.draw(logoutBtnText);
            }
        } else {
            // Other tabs - show default status
            sf::Text status("Status: " + vm.GetStatus(), font, 14u);
            status.setFillColor(sf::Color(100, 100, 100));
            status.setPosition(sidebarWidth + 20.f, 70.f);
            window.draw(status);
        }
        
        window.display();
    }
#else
    std::cout << "-- Console fallback GUI --\n";
    std::cout << "Press enter to start extraction (skeleton)" << std::endl;
    std::string dummy;
    std::getline(std::cin, dummy);
    vm.StartExtraction("/path/to/file.pdf");
    std::cout << "Status: " << vm.GetStatus() << std::endl;
    std::cout << "Result: " << vm.GetLastResult() << std::endl;
#endif
}
