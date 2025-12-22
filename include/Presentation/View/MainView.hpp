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
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            
            // Pass events to UI
            sidebar->handleEvent(event);
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
        
        // Draw status info
        sf::Text status("Status: " + vm.GetStatus(), font, 14u);
        status.setFillColor(sf::Color(100, 100, 100));
        status.setPosition(sidebarWidth + 20.f, 70.f);
        window.draw(status);
        
        sf::Text result("Last Result: " + vm.GetLastResult(), font, 12u);
        result.setFillColor(sf::Color(100, 100, 100));
        result.setPosition(sidebarWidth + 20.f, 100.f);
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
