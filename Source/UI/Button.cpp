// by Dmitry Kolontay

#include "Button.h"
#include <cassert>
Button::Button(DragonJumpFramework& _framework, 
    const Vector2Df& position, const std::string& spritePath) :
    Drawable(_framework, position), bIsPressed{ false },
    sprites{ framework.GetNumberedSprites(spritePath) }
{
    if (sprites.size() == 2 && framework.IsInScreenArea(
        position, sprites.at(0).sprite, true, true)) {
        for (auto& sprite : sprites) {
            sprite.offset *= 0.5f;
        }
    }
    else {
        bIsActive = false;
    }
    assert(bIsActive && "Button : initialization failed");
}

bool Button::DrawIfActive_Internal()
{
    if (Vector2Df mousePos{ framework.GetMousePosition() }; 
        mousePos.IsPointInsideRectangle(
            position - sprites.at(1).offset, 
            position + sprites.at(1).offset)) {
        sprites.at(1).Draw(position);
    }
    else {
        sprites.at(0).Draw(position);
    }
    return true;
}

bool Button::Reactivate(const Vector2Df& pos)
{
    position = pos;
    bIsActive = sprites.size() == 2 && 
        framework.IsInScreenArea(position, sprites.at(0).sprite, true, true);
    return bIsActive;
}

bool Button::OnMouseButtonDown(const Vector2D& clickPos)
{
    assert(bIsActive);
    if (bIsPressed)
        return false;
    if (Vector2Df pos{ clickPos }; pos.IsPointInsideRectangle(
        position - sprites.at(0).offset, position + sprites.at(0).offset)) {
        bIsPressed = true;
        return true;
    }
    else {
        return false;
    }
}

bool Button::OnMouseButtonUp(const Vector2D& clickPos)
{
    assert(bIsActive);
    if (!bIsPressed)
        return false;
    if (Vector2Df pos{ clickPos }; pos.IsPointInsideRectangle( 
        position - sprites.at(1).offset, position + sprites.at(1).offset)) {
        std::invoke(onClicked);
        bIsPressed = false;
        return true;
    }
    else {
        return false;
    }
}

