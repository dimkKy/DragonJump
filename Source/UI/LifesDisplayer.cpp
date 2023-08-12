// by Dmitry Kolontay

#include "LifesDisplayer.h"
#include "SpriteLoader.h"
#include <cassert>

LifesDisplayer::LifesDisplayer(DragonJumpFramework& _framework, const Vector2Df& pos) :
    Drawable(_framework, pos), sprite{ framework.GetSpriteInfo(SpritePaths::lifeIcon)}
{
    if (!sprite || position.y >= framework.GetSize().y ||
        position.x + sprite.offset.x >= framework.GetSize().x) {
        bIsActive = false;
        sprite.Reset();
    }
    else {
        sprite.offset.y = 0.f;
    }
    assert(bIsActive && "LifesDisplayer init unsuccessfull");
}

bool LifesDisplayer::Reactivate(const Vector2Df& pos) &
{
    position = pos;
    if (!sprite || position.y >= framework.GetSize().y ||
        position.x + sprite.offset.x >= framework.GetSize().x) {
        bIsActive = false;
    }
	else {
		bIsActive = true;
	}
	return bIsActive;
}

bool LifesDisplayer::DrawIfActive_Internal()
{
    int lifesCount{ framework.GetLifesLeft() };
    for (auto i{ 1 }; i <= lifesCount; ++i) {
        sprite.Draw(position.x + sprite.offset.x * i, position.y);
    }
    return false;
}
