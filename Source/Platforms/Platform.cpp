// by Dmitry Kolontay

#include "Platform.h"
#include "DragonJumpFramework.h"
#include "SpriteLoader.h"
#include <cassert>

Platform::Platform(DragonJumpFramework& _framework, const Vector2Df& pos) :
	Drawable(_framework, pos)
{
	if (framework.GetSpriteInfo(SpritePaths::defaultPlatform, defaultSprite)) {
		defaultSprite.offset *= 0.5f;
		collisionInfo.halfSize = defaultSprite.offset;
		collisionInfo.halfSize.x *= 0.9f;
	}
	else {
		bIsActive = false;
	}
	assert(bIsActive && "Ability init unsuccessfull");
}

bool Platform::Reactivate(const Vector2Df& pos) &
{
	position = pos;
	bIsActive = true;
	bool test{ IsActive() };
	assert(bIsActive && "Platform reported faied reactivation");
	return bIsActive;
}

bool Platform::IsActive()
{
	if (Collidable::IsActive()) {
		bIsActive = framework.IsInScreenArea(position, defaultSprite.sprite);
	}
	return bIsActive;
}

bool Platform::DrawIfActive_Internal()
{
	if (position.y + defaultSprite.offset.y > +0.f) {
		defaultSprite.Draw(position);
		return true;
	}
	else
		return false;
}
