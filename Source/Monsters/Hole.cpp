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
		collisionInfo.radiusSquared = sprite.offset.LengthSquared() * 0.25f;
	}
	else {
		bIsActive = false;
	}
	assert(bIsActive && "Hole init unsuccessfull");
}

bool Hole::Reactivate(const Vector2Df& newPosition)
{
	if (Collidable::IsActive()) {
		position = newPosition;
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

/*void Hole::ReceiveCollision(Collidable& other)
{
	if (PlayerDoodle * playerDoodle{ dynamic_cast<PlayerDoodle*>(&other) }) {
		playerDoodle->StartFallingInTheHole(*this);
	}
}*/

bool Hole::IsActive()
{
	if (Collidable::IsActive()) {
		//check out of bounds
		bIsActive = framework.IsInScreenArea(position, 
			framework.GetSpriteSize(sprite.sprite));
	}
	return bIsActive;
}
