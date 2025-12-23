#pragma once

#include "Widget.h"
#include <vector>
#include <memory>

namespace UI {

struct RibbonItem {
    std::string name;
    std::function<void()> callback;
};

class Sidebar {
public:
    Sidebar(const sf::Font &font, float width = 250.f, float height = 700.f);
    
    void draw(sf::RenderWindow &window);
    void handleEvent(const sf::Event &event);
    
    void toggle();
    bool isExpanded() const { return isExpanded_; }
    
    void addRibbon(const std::string &name);
    void addItemToRibbon(const std::string &ribbonName, const RibbonItem &item);
    
    void setHeight(float height) { 
        height_ = height;
        background_.setSize(sf::Vector2f(getWidth(), height_));
    }
    
    float getWidth() const { return isExpanded_ ? width_ : collapsedWidth_; }
    float getHeight() const { return height_; }
    
    /// Get which ribbon was clicked (0-5 for the 6 ribbons, -1 if none)
    int getLastClickedRibbon() const { return lastClickedRibbon_; }
    void resetClickedRibbon() { lastClickedRibbon_ = -1; }
    
private:
    const sf::Font &font_;
    float width_;
    float height_;
    float collapsedWidth_ = 60.f;
    bool isExpanded_ = true;
    
    struct Ribbon {
        std::string name;
        char icon; // First letter for icon
        bool isOpen = false;
        std::vector<RibbonItem> items;
        bool hovered = false;
    };
    
    std::vector<Ribbon> ribbons_;
    
    // UI elements
    sf::RectangleShape background_;
    Button toggleButton_;
    std::vector<Button> ribbonButtons_;
    std::vector<Button> itemButtons_;
    int hoveredRibbonIndex_ = -1;
    int lastClickedRibbon_ = -1;
    
    void updateLayout();
};

} // namespace UI
