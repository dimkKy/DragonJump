// by Dmitry Kolontay

#include "Projectile.h"
#include "DragonJumpFramework.h"
#include "PlayerDoodle.h"
#include "Monster.h"
#include "SpriteLoader.h"
#include <cassert>

//https://bugs.llvm.org/show_bug.cgi?id=34516#c1

Projectile::Projectile(DragonJumpFramework& _framework, const Vector2Df& target) :
	Drawable(_framework, _framework.GetPlayerDoodle()->GetMouthGlobalPos()), 
	Tickable((target - position).ToUnit() * projectileSpeed)
{
	if (framework.GetSpriteInfo(SpritePaths::projectile, sprite)) {
		collisionInfo.radiusSquared = sprite.offset.LengthSquared();
		sprite.offset *= 0.5f;
	}
	else {
		bIsActive = false;
	}
	assert(bIsActive && "Ability init unsuccessfull");
}

bool Projectile::Reactivate(const Vector2Df& target) &
{
	if (collisionInfo) {
		position = framework.GetPlayerDoodle()->GetMouthGlobalPos();
		velocity = (target - position).ToUnit() * projectileSpeed;
		bIsActive = true;
		return IsActive();
	}
	else {
		return false;
	}
}

bool Projectile::IsActive()
{
	if (Collidable::IsActive()) {
		bIsActive = framework.IsInScreenArea(position, 
			framework.GetSpriteSize(sprite.sprite), true, true);
	}
	return bIsActive;
}

void Projectile::ReceiveCollision(CollidableBase& other)
{
	if (other.GetCollisionChannel() == CC::Monster) {
		bIsActive = false;
	}
}

bool Projectile::DrawIfActive_Internal()
{
	if (position.y + sprite.offset.y > +0.f) {
		sprite.Draw(position);
#ifndef ORIGINAL_SHOOTING
		if (int additionalDraw{ framework.DoesSpriteTouchBorder(position, sprite.sprite) }) {
			float posX{ position.x };
			position.x -= framework.GetSize().x * additionalDraw;
			sprite.Draw(position);
			position.x = posX;
		}
#endif // !ORIGINAL_SHOOTING
		return true;
	}
	else {
		return false;
	}
}

void Projectile::ReceiveTick(float deltaTime) &
{
	assert(IsActive() && "Projectile: tick received while inactive");
	position += velocity * deltaTime;
#ifndef ORIGINAL_SHOOTING
	position.x = std::fmodf(position.x,
		static_cast<float>(framework.GetSize().x));
	if (position.x < 0.f) {
		position.x += framework.GetSize().x;
	}
#endif // !ORIGINAL_SHOOTING

}
