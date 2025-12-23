#include "../../include/UI/Sidebar.h"

namespace UI {

Sidebar::Sidebar(const sf::Font &font, float width, float height)
    : font_(font), width_(width), height_(height), toggleButton_("☰", font)
{
    background_.setSize(sf::Vector2f(width, height));
    background_.setFillColor(sf::Color(45, 45, 48)); // Dunkles Grau
    
    toggleButton_.setPosition(8.f, 8.f);
    toggleButton_.setSize(44.f, 44.f);
    toggleButton_.setBackgroundColor(sf::Color(60, 60, 63));
    toggleButton_.setHoverColor(sf::Color(80, 80, 85));
    toggleButton_.setCallback([this]() { toggle(); });
    
    // Initialize standard ribbons with icons (first letters)
    addRibbon("Home");
    addRibbon("Upload");
    addRibbon("Extraktion");
    addRibbon("Admin Panel");
    addRibbon("Einstellungen");
    addRibbon("Profil");
    
    // Set icons as first letters
    for (size_t i = 0; i < ribbons_.size(); ++i) {
        ribbons_[i].icon = ribbons_[i].name[0];
    }
}

void Sidebar::draw(sf::RenderWindow &window)
{
    background_.setSize(sf::Vector2f(getWidth(), height_));
    window.draw(background_);
    
    // Draw toggle button
    toggleButton_.draw(window);
    
    if (isExpanded_) {
        // Draw expanded menu with full names and items
        float yOffset = 65.f;
        for (size_t i = 0; i < ribbons_.size(); ++i) {
            auto &ribbon = ribbons_[i];
            
            // Draw ribbon button background
            sf::RectangleShape ribbonBg(sf::Vector2f(width_ - 10.f, 30.f));
            ribbonBg.setPosition(5.f, yOffset);
            ribbonBg.setFillColor(ribbon.hovered ? sf::Color(70, 70, 73) : sf::Color(55, 55, 58));
            window.draw(ribbonBg);
            
            // Draw ribbon text
            sf::Text ribbonText(ribbon.name, font_, 13u);
            ribbonText.setFillColor(sf::Color(220, 220, 220));
            ribbonText.setPosition(15.f, yOffset + 5.f);
            window.draw(ribbonText);
            
            yOffset += 35.f;
            
            // Draw items if ribbon is open
            if (ribbon.isOpen) {
                for (auto &item : ribbon.items) {
                    sf::RectangleShape itemBg(sf::Vector2f(width_ - 15.f, 25.f));
                    itemBg.setPosition(10.f, yOffset);
                    itemBg.setFillColor(sf::Color(50, 50, 53));
                    window.draw(itemBg);
                    
                    sf::Text itemText("► " + item.name, font_, 11u);
                    itemText.setFillColor(sf::Color(180, 180, 180));
                    itemText.setPosition(20.f, yOffset + 4.f);
                    window.draw(itemText);
                    yOffset += 28.f;
                }
            }
        }
    } else {
        // Draw collapsed menu with icons (first letters)
        float yOffset = 65.f;
        for (size_t i = 0; i < ribbons_.size(); ++i) {
            auto &ribbon = ribbons_[i];
            
            // Draw icon button background
            sf::RectangleShape iconBg(sf::Vector2f(45.f, 45.f));
            iconBg.setPosition(7.f, yOffset);
            iconBg.setFillColor(ribbon.hovered ? sf::Color(70, 70, 73) : sf::Color(55, 55, 58));
            window.draw(iconBg);
            
            // Draw icon (first letter)
            std::string iconStr(1, ribbon.icon);
            sf::Text iconText(iconStr, font_, 20u);
            iconText.setFillColor(sf::Color(200, 200, 200));
            iconText.setPosition(17.f, yOffset + 8.f);
            window.draw(iconText);
            
            yOffset += 52.f;
        }
    }
}

void Sidebar::handleEvent(const sf::Event &event)
{
    toggleButton_.handleEvent(event);
    
    if (event.type == sf::Event::MouseMoved) {
        float yOffset = 65.f;
        int newHovered = -1;
        
        for (size_t i = 0; i < ribbons_.size(); ++i) {
            float itemHeight = isExpanded_ ? 35.f : 52.f;
            if (event.mouseMove.y >= yOffset && event.mouseMove.y < yOffset + itemHeight &&
                event.mouseMove.x < getWidth()) {
                newHovered = i;
                break;
            }
            yOffset += itemHeight;
        }
        
        // Update hover states
        for (size_t i = 0; i < ribbons_.size(); ++i) {
            ribbons_[i].hovered = (static_cast<int>(i) == newHovered);
        }
    }
    
    // Handle ribbon clicks for expanding/collapsing
    if (event.type == sf::Event::MouseButtonPressed) {
        float yOffset = 65.f;
        int ribbonIndex = 0;
        
        for (auto &ribbon : ribbons_) {
            float itemHeight = isExpanded_ ? 35.f : 52.f;
            if (event.mouseButton.y >= yOffset && event.mouseButton.y < yOffset + itemHeight &&
                event.mouseButton.x < getWidth()) {
                ribbon.isOpen = !ribbon.isOpen;
                lastClickedRibbon_ = ribbonIndex;
                break;
            }
            yOffset += itemHeight;
            
            if (ribbon.isOpen) {
                for (auto &item : ribbon.items) {
                    if (event.mouseButton.y >= yOffset && event.mouseButton.y < yOffset + 28.f &&
                        event.mouseButton.x < getWidth()) {
                        if (item.callback) item.callback();
                    }
                    yOffset += 28.f;
                }
            }
            ++ribbonIndex;
        }
    }
}

void Sidebar::toggle()
{
    isExpanded_ = !isExpanded_;
}

void Sidebar::addRibbon(const std::string &name)
{
    ribbons_.push_back(Ribbon{name, name[0], false, {}, false});
}

void Sidebar::addItemToRibbon(const std::string &ribbonName, const RibbonItem &item)
{
    for (auto &ribbon : ribbons_) {
        if (ribbon.name == ribbonName) {
            ribbon.items.push_back(item);
            break;
        }
    }
}

} // namespace UI
