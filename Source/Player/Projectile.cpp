#include "Projectile.h"
#include "DragonJumpFramework.h"
#include "PlayerDoodle.h"
#include "Monster.h"
#include "SpriteLoader.h"
#include <cassert>

//https://bugs.llvm.org/show_bug.cgi?id=34516#c1

const float Projectile::projectileSpeed = 500.f;

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

bool Projectile::Reactivate(const Vector2Df& target)
{
	if (collisionInfo) {
		position = framework.GetPlayerDoodle()->GetMouthGlobalPos();
		currentVelocity = (target - position).ToUnit() * projectileSpeed;
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
	if (other.GetCollisionChannel() == CollisionChannel::CC_Monster) {
		bIsActive = false;
	}
}

bool Projectile::DrawIfActive_Internal()
{
	if (position.y + sprite.offset.y > +0.f) {
		sprite.Draw(position);
		return true;
	}
	else {
		return false;
	}
}

void Projectile::ReceiveTick(float deltaTime)
{
	assert(IsActive() && "Projectile: tick received while inactive");
	position += currentVelocity * deltaTime;
}
