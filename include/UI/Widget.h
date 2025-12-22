#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <functional>

namespace UI {

// Base Widget class
class Widget {
public:
    virtual ~Widget() = default;
    
    virtual void draw(sf::RenderWindow &window) = 0;
    virtual void handleEvent(const sf::Event &event) = 0;
    
    void setPosition(float x, float y) { 
        position_ = sf::Vector2f(x, y); 
    }
    
    void setSize(float width, float height) { 
        size_ = sf::Vector2f(width, height); 
    }
    
    sf::Vector2f getPosition() const { return position_; }
    sf::Vector2f getSize() const { return size_; }
    
protected:
    sf::Vector2f position_;
    sf::Vector2f size_;
};

// Button widget
class Button : public Widget {
public:
    Button(const std::string &label, const sf::Font &font);
    
    void draw(sf::RenderWindow &window) override;
    void handleEvent(const sf::Event &event) override;
    
    void setCallback(std::function<void()> callback) { callback_ = callback; }
    void setBackgroundColor(const sf::Color &color) { bgColor_ = color; }
    void setHoverColor(const sf::Color &color) { hoverColor_ = color; }
    void setTextColor(const sf::Color &color) { textColor_ = color; }
    
    bool isHovered() const { return isHovered_; }
    
private:
    std::string label_;
    const sf::Font &font_;
    sf::Color bgColor_ = sf::Color(100, 100, 100);
    sf::Color hoverColor_ = sf::Color(150, 150, 150);
    sf::Color textColor_ = sf::Color::White;
    bool isHovered_ = false;
    std::function<void()> callback_;
};

// Rectangle shape helper
class RectShape : public Widget {
public:
    RectShape(float width, float height, const sf::Color &color = sf::Color::White);
    
    void draw(sf::RenderWindow &window) override;
    void handleEvent(const sf::Event &event) override {}
    
    void setFillColor(const sf::Color &color) { shape_.setFillColor(color); }
    
private:
    sf::RectangleShape shape_;
};

} // namespace UI
