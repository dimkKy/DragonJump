#include "Monster.h"
#include "Projectile.h"
#include "PlayerDoodle.h"

/*Monster::Monster(DragonJumpFramework& _framework, const float& positionX, const float& positionY) :
    Drawable(_framework, positionX, positionY)
{
	/*if (spriteInfo.offset.x > spriteInfo.offset.y) {
		if (static_cast<float>(spriteInfo.offset.x) / static_cast<float>(spriteInfo.offset.y) > 1.1f) {
			collisionSize = Vector2Df{ static_cast<float>(spriteInfo.offset.x), static_cast<float>(spriteInfo.offset.y) };
		}
		else {
			collisionSize = Vector2Df{ 0.f, static_cast<float>(spriteInfo.offset.y) * static_cast<float>(spriteInfo.offset.x) };
		}
	}
	else {
		if (static_cast<float>(spriteInfo.offset.y) / static_cast<float>(spriteInfo.offset.x) > 1.1f) {
			collisionSize = Vector2Df{ static_cast<float>(spriteInfo.offset.x), static_cast<float>(spriteInfo.offset.y) };
		}
		else {
			collisionSize = Vector2Df{ 0.f, static_cast<float>(spriteInfo.offset.y) * static_cast<float>(spriteInfo.offset.x) };
		}
	}
	defaultSpriteInfo.offset.x *= 0.5f;
	defaultSpriteInfo.offset.y *= 0.5f;*
}*/

/*bool Monster::DrawIfActive(int cameraVerticalOffset)
{
    return false;
}*/

/*Vector2Df Monster::GetCollisionSize() const
{
    return collisionSize;
}*/

/*CollisionChannel Monster::GetCollisionChannel() const
{
    return CollisionChannel::CC_Monster;
}*/

/*bool Monster::GetCollisionResponse(CollisionChannel channel) const
{
    return channel == CollisionChannel::CC_Projectile || channel == CollisionChannel::CC_Player;
}*/

/*void Monster::ReceiveCollision(Collidable& other)
{
    if (Projectile* projectile{ dynamic_cast<Projectile*>(&other) }) {
        //TODO
    }
    else {
        if (PlayerDoodle* playerDoodle{ dynamic_cast<PlayerDoodle*>(&other) }) {
            //TODO
        }
    }
}*/

/*Monster::Monster(const SpriteInfo& spriteInfo) : defaultSpriteInfo{ spriteInfo }
{
	if (spriteInfo.offset.x > spriteInfo.offset.y) {
		if (static_cast<float>(spriteInfo.offset.x) / static_cast<float>(spriteInfo.offset.y) > 1.1f) {
			collisionSize = Vector2Df{ static_cast<float>(spriteInfo.offset.x), static_cast<float>(spriteInfo.offset.y) };
		}
		else {
			collisionSize = Vector2Df{ 0.f, static_cast<float>(spriteInfo.offset.y) * static_cast<float>(spriteInfo.offset.x) };
		}
	}
	else {
		if (static_cast<float>(spriteInfo.offset.y) / static_cast<float>(spriteInfo.offset.x) > 1.1f) {
			collisionSize = Vector2Df{ static_cast<float>(spriteInfo.offset.x), static_cast<float>(spriteInfo.offset.y) };
		}
		else {
			collisionSize = Vector2Df{ 0.f, static_cast<float>(spriteInfo.offset.y) * static_cast<float>(spriteInfo.offset.x) };
		}
	}
	defaultSpriteInfo.offset.x *= 0.5f;
	defaultSpriteInfo.offset.y *= 0.5f;
}*/

