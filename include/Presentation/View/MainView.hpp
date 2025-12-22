#pragma once

#include "../ViewModel/MainViewModel.h"
#include "../../UI/Sidebar.h"
#include <iostream>
#include <memory>

#ifdef USE_SFML
#include <SFML/Graphics.hpp>
#endif

using namespace Presentation::ViewModel;

void RunGui(MainViewModel &vm)
{
#ifdef USE_SFML
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(1200, 700)), "Text Extraction - MVVM (SFML)");
    sf::Font font;
    if (!font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
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
    
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            
            // Pass events to UI
            sidebar->handleEvent(*event);
        }
        
        window.clear(sf::Color::White);
        
        // Draw sidebar
        sidebar->draw(window);
        
        // Draw main content area
        float sidebarWidth = sidebar->getWidth();
        sf::RectangleShape contentArea(sf::Vector2f(1200.f - sidebarWidth, 700.f));
        contentArea.setPosition(sf::Vector2f(sidebarWidth, 0.f));
        contentArea.setFillColor(sf::Color(240, 240, 240));
        window.draw(contentArea);
        
        // Draw header text
        sf::Text header(font, "Text Extraction System", 28u);
        header.setFillColor(sf::Color::Black);
        header.setPosition(sf::Vector2f(sidebarWidth + 20.f, 20.f));
        window.draw(header);
        
        // Draw status info
        sf::Text status(font, "Status: " + vm.GetStatus(), 14u);
        status.setFillColor(sf::Color(100, 100, 100));
        status.setPosition(sf::Vector2f(sidebarWidth + 20.f, 70.f));
        window.draw(status);
        
        sf::Text result(font, "Last Result: " + vm.GetLastResult(), 12u);
        result.setFillColor(sf::Color(100, 100, 100));
        result.setPosition(sf::Vector2f(sidebarWidth + 20.f, 100.f));
        window.draw(result);
        
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
