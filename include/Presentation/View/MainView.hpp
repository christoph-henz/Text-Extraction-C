#pragma once

#include "../ViewModel/MainViewModel.h"
#include "../../UI/Sidebar.h"
#include "../../Services/ApiService.h"
#include "../../Services/LoginService.h"
#include <iostream>
#include <memory>
#include <sstream>

#ifdef USE_SFML
#include <SFML/Graphics.hpp>
#endif

using namespace Presentation::ViewModel;

void RunGui(Presentation::ViewModel::MainViewModel &vm)
{
#ifdef USE_SFML
    // Initialize API Service with default
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
    
    // API State
    std::string apiUrl = Services::ApiService::GetApiUrl();
    std::string apiResponse = "";
    int activeTab = 0; // 0 = Home, 4 = Einstellungen, 5 = Profil
    bool showApiUrlInput = false;
    std::string urlInput = apiUrl;
    
    // Login State
    std::string loginUsername = "";
    std::string loginPassword = "";
    std::string loginError = "";
    bool isLoginInputMode = true; // false = show user data
    Services::LoginInfo userInfo{"Guest", "", "User", ""};
    bool showPasswordInput = false; // Show actual password chars or dots
    
    // Initialize with logged-in state if user exists
    if (userAlreadyLoggedIn) {
        isLoginInputMode = false;
        auto info = Services::LoginService::GetLoginInfo();
        if (info.has_value()) {
            userInfo = info.value();
        }
    }
    
    while (window.isOpen()) {
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
                // Determine if username or password field is active (simple tab detection)
                // For now, username first, then password after first Enter press
                // This is a simplified approach - a proper implementation would use UI focus states
                static bool usernameActive = true;
                
                if (event.text.unicode == 9) { // Tab key - switch fields
                    usernameActive = !usernameActive;
                } else if (event.text.unicode == 8) { // Backspace
                    if (usernameActive && !loginUsername.empty()) {
                        loginUsername.pop_back();
                    } else if (!usernameActive && !loginPassword.empty()) {
                        loginPassword.pop_back();
                    }
                } else if (event.text.unicode >= 32 && event.text.unicode < 127) {
                    if (usernameActive) {
                        loginUsername += static_cast<char>(event.text.unicode);
                    } else {
                        loginPassword += static_cast<char>(event.text.unicode);
                    }
                }
            }
        }
        
        // Check if a ribbon was clicked to change tab
        int clickedRibbon = sidebar->getLastClickedRibbon();
        if (clickedRibbon >= 0) {
            // Map ribbons to tabs: Home=0, Upload=1, Extraktion=2, Admin=3, Einstellungen=4, Profil=5
            activeTab = clickedRibbon;
            sidebar->resetClickedRibbon();
            
            // Handle logout if Profil Logout is clicked
            if (clickedRibbon == 5 && !isLoginInputMode) {
                Services::LoginService::ClearLogin();
                isLoginInputMode = true;
                loginUsername = "";
                loginPassword = "";
                loginError = "";
            }
        }
        
        window.clear(sf::Color::White);
        
        // Draw sidebar
        sidebar->draw(window);
        
        // Draw main content area
        float sidebarWidth = sidebar->getWidth();
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
                    apiResponse = connected ? "✓ Connected successfully!" : "✗ Connection failed!";
                }
            }
            
            if (!apiResponse.empty()) {
                sf::Text connStatus(apiResponse, font, 12u);
                connStatus.setFillColor(apiResponse.find("✓") != std::string::npos ? sf::Color(50, 150, 50) : sf::Color(200, 50, 50));
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
                userBox.setOutlineColor(sf::Color::Black);
                userBox.setOutlineThickness(1.f);
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
                passBox.setOutlineColor(sf::Color::Black);
                passBox.setOutlineThickness(1.f);
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
                
                // Check for login button click
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.x >= sidebarWidth + 50.f && event.mouseButton.x <= sidebarWidth + 150.f &&
                        event.mouseButton.y >= 300.f && event.mouseButton.y <= 335.f) {
                        // Perform login
                        Services::HttpResponse resp = Services::ApiService::Login(loginUsername, loginPassword);
                        if (resp.isSuccess) {
                            // Save login info
                            Services::LoginService::SaveLogin(loginUsername, resp.user_email, resp.user_role, "user");
                            Services::ApiService::SetAuthCredentials(loginUsername, loginPassword);
                            auto info = Services::LoginService::GetLoginInfo();
                            if (info.has_value()) {
                                userInfo = info.value();
                            }
                            isLoginInputMode = false;
                            loginError = "";
                        } else {
                            loginError = "Login failed: " + std::to_string(resp.statusCode);
                        }
                    }
                }
                
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
                
                // Check for logout button click
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.x >= sidebarWidth + 50.f && event.mouseButton.x <= sidebarWidth + 170.f &&
                        event.mouseButton.y >= 280.f && event.mouseButton.y <= 315.f) {
                        Services::LoginService::ClearLogin();
                        isLoginInputMode = true;
                        loginUsername = "";
                        loginPassword = "";
                        loginError = "";
                    }
                }
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
