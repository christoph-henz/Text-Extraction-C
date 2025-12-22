#include "../../include/Presentation/ViewModel/MainViewModel.h"
#include <iostream>

#ifdef USE_SFML
#include <SFML/Graphics.hpp>
#endif

using namespace Presentation::ViewModel;

void RunGui(MainViewModel &vm)
{
#ifdef USE_SFML
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(640, 480)), "Text Extraction - MVVM (SFML)");
    sf::Font font;
    if (!font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        // try default
    }
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (event->is<sf::Event::KeyPressed>() && event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Space) {
                vm.StartExtraction("/path/to/file.pdf");
            }
        }
        window.clear(sf::Color::Black);
        sf::Text status(font);
        status.setString("Status: " + vm.GetStatus());
        status.setCharacterSize(18u);
        status.setFillColor(sf::Color::White);
        status.setPosition(sf::Vector2f(10.f, 10.f));
        window.draw(status);
        sf::Text result(font);
        result.setString(vm.GetLastResult());
        result.setCharacterSize(14u);
        result.setFillColor(sf::Color::White);
        result.setPosition(sf::Vector2f(10.f, 40.f));
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
