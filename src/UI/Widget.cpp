#include "../../include/UI/Widget.h"
#include <iostream>

namespace UI {

// Button implementation
Button::Button(const std::string &label, const sf::Font &font)
    : label_(label), font_(font)
{
    size_ = sf::Vector2f(120.f, 40.f);
}

void Button::draw(sf::RenderWindow &window)
{
    // Draw background rectangle
    sf::RectangleShape bg(size_);
    bg.setPosition(position_);
    bg.setFillColor(isHovered_ ? hoverColor_ : bgColor_);
    window.draw(bg);
    
    // Draw border
    sf::RectangleShape border(size_);
    border.setPosition(position_);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::Black);
    border.setOutlineThickness(1.f);
    window.draw(border);
    
    // Draw text
    sf::Text text(label_, font_, 14u);
    text.setFillColor(textColor_);
    text.setPosition(position_.x + 10.f, position_.y + 10.f);
    window.draw(text);
}

void Button::handleEvent(const sf::Event &event)
{
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
        isHovered_ = (mousePos.x >= position_.x && mousePos.x < position_.x + size_.x &&
                      mousePos.y >= position_.y && mousePos.y < position_.y + size_.y);
    }
    
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
            if (mousePos.x >= position_.x && mousePos.x < position_.x + size_.x &&
                mousePos.y >= position_.y && mousePos.y < position_.y + size_.y) {
                if (callback_) callback_();
            }
        }
    }
}

// RectShape implementation
RectShape::RectShape(float width, float height, const sf::Color &color)
{
    size_ = sf::Vector2f(width, height);
    shape_.setSize(size_);
    shape_.setFillColor(color);
}

void RectShape::draw(sf::RenderWindow &window)
{
    shape_.setPosition(position_);
    window.draw(shape_);
}

} // namespace UI
