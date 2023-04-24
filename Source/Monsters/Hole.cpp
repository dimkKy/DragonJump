// by Dmitry Kolontay

#include "Hole.h"
#include "PlayerDoodle.h"
#include "SpriteLoader.h"
#include <cassert>

Hole::Hole(DragonJumpFramework& _framework, const Vector2Df& _position) :
	Drawable(_framework, _position), Monster(MonsterType::MT_Hole)
{
	if (framework.GetSpriteInfo(SpritePaths::hole, sprite)) {
		sprite.offset *= 0.5f;
		//?!
		position.y += sprite.offset.y * 1.1f;
		collisionInfo.radiusSquared = sprite.offset.LengthSquared() * 0.27f;
	}
	else {
		bIsActive = false;
	}
	assert(bIsActive && "Hole init unsuccessfull");
}

bool Hole::Reactivate(const Vector2Df& newPosition)
{
	if (Collidable::IsActive()) {
		position.x = newPosition.x;
		//?!
		position.y = newPosition.y + sprite.offset.y * 1.1f;
		bIsActive = true;
	}
	return bIsActive;
}

bool Hole::DrawIfActive_Internal()
{
	if (position.y + sprite.offset.y > +0.f) {
		sprite.Draw(position);
		return true;
	}
	else {
		return false;
	}
}

bool Hole::IsActive()
{
	if (Collidable::IsActive()) {
		//check out of bounds
		bIsActive = framework.IsInScreenArea(position, 
			framework.GetSpriteSize(sprite.sprite));
	}
	return bIsActive;
}
