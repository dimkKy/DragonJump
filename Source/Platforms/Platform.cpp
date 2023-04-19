#include "Platform.h"
#include "DragonJumpFramework.h"
#include "SpriteLoader.h"
#include <cassert>

Platform::Platform(DragonJumpFramework& _framework, const Vector2Df& pos) :
	Drawable(_framework, pos)
{
	if (framework.GetSpriteInfo(SpritePaths::defaultPlatform, defaultSprite)) {
		collisionInfo.size = defaultSprite.offset;
		defaultSprite.offset * 0.5f;
	}
	else {
		bIsActive = false;
	}
	assert(bIsActive && "Ability init unsuccessfull");
}

bool Platform::Reactivate(const Vector2Df& pos)
{
	position = pos;
	bIsActive = true;
	return IsActive();
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



