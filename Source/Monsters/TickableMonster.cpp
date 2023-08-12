// by Dmitry Kolontay

#include "TIckableMonster.h"
#include "Projectile.h"
#include "SpriteLoader.h"
#include <cassert>
#include <numbers>

TickableMonster::TickableMonster(DragonJumpFramework& _framework, const Vector2Df& _position, MonsterType _type) :
	Drawable(_framework, _position), Monster(_type), 
	spriteToDraw{ 0 }, dyingAnimationTime{ -1.f }, idleAnimationTime{ 0.f }
{
	std::string path;
	int desiredSpriteNumber{ 2 };
	switch (type) {
	case MonsterType::MT_Static:
		path = SpritePaths::staticMonster;
		break;
	case MonsterType::MT_Movable:
		path = SpritePaths::movableMonster;
		velocity.x = movableMonsterXSpeed;
		break;
	default:
		break;
	}
	framework.GetNumberedSprites(path, spriteInfos);
	if (spriteInfos.size() == desiredSpriteNumber) {
		for (auto& info : spriteInfos) {
			info.offset *= 0.5f;
		}
		collisionInfo.halfSize = spriteInfos.at(0).offset;
		//?!
		position.y -= collisionInfo.halfSize.y * 1.1f;
	}
	else {
		bIsActive = false;
		type = MonsterType::MT_COUNT;
	}
	assert(bIsActive && "TickableMonster : sprites init failed");
}

Vector2Df TickableMonster::GetPosition() const
{
	return { position.x, position.y + GetAnimOffsetY()};
}

bool TickableMonster::DrawIfActive_Internal()
{	
	if (position.y + spriteInfos.at(0).offset.y > +0.f) {
		spriteInfos.at(spriteToDraw).Draw(
			position.x, position.y + GetAnimOffsetY());
		return true;
	}
	else
		return false;
}

float TickableMonster::GetAnimOffsetY() const
{
	if (dyingAnimationTime < 0.f) {
		return spriteInfos.at(0).offset.y * idleAnimAmplitudeMp *
			sinf(idleAnimationTime / idleAnimationPeriod * std::numbers::pi_v<float>);
	}
	else {
		return 0.f;
	}
}

void TickableMonster::ReceiveCollision(CollidableBase& other)
{
	if (Projectile * projectile{ dynamic_cast<Projectile*>(&other) }) {
		if (dyingAnimationTime >= 0.f) {
			return;
		}
		dyingAnimationTime = 0.f;
		velocity = 0.f;
		if (type == MonsterType::MT_Static) {
			spriteToDraw = static_cast<int>(spriteInfos.size()) - 1;
		}
		framework.IncreaseMonstersKilledCounter();
		position = GetPosition();
	}
}

bool TickableMonster::IsActive()
{
	if (Collidable::IsActive()) {
		if (dyingAnimationTime > dyingAnimationDuration && velocity.y == 0.f) {
			bIsActive = false;
			return bIsActive;
		}
		bIsActive = framework.IsInScreenArea(position, 
			spriteInfos.at(spriteToDraw).sprite);
	}
	return bIsActive;
}

bool TickableMonster::CanBeSteppedOn() const
{
	return dyingAnimationTime < 0.f;
}

void TickableMonster::OnJumpFrom(PlayerDoodle& other)
{
	if (dyingAnimationTime < 0.f) {
		dyingAnimationTime = 0.f;
		velocity.x = 0.f;
		velocity.y = dyingVerticalSpeed;
		framework.IncreaseMonstersKilledCounter();
	}
}

void TickableMonster::ReceiveTick(float deltaTime) &
{
	if (dyingAnimationTime >= 0.f) {
		dyingAnimationTime += deltaTime;
		if (velocity.y > 0.f) {
			if (dyingAnimationTime >= knockoutAnimDuration) {
				dyingAnimationTime = std::fmodf(dyingAnimationTime, knockoutAnimDuration);
			}
			position.y += velocity.y * deltaTime;
		}
		else {
			if (dyingAnimationTime > dyingAnimationDuration) {
				bIsActive = false;
			}
		}
		return;
	}
	position += velocity * deltaTime;
	idleAnimationTime = std::fmodf(idleAnimationTime + deltaTime, idleAnimationPeriod);
	if (velocity.x) {
		float oldPos = { position.x };
		position.x = std::clamp(position.x, spriteInfos.at(spriteToDraw).offset.x, 
			framework.GetSize().x - spriteInfos.at(spriteToDraw).offset.x);
		if (oldPos != position.x) {
			velocity.x *= -1;
			spriteToDraw = (velocity.x < 0.f);
		}
	}
}

bool TickableMonster::Reactivate(const Vector2Df& pos) &
{
	if (type == MonsterType::MT_COUNT) {
		bIsActive = false;
		return false;
	}
	dyingAnimationTime = -1.f;
	position.x = pos.x;
	position.y = pos.y - collisionInfo.halfSize.y * 1.1f;
	switch (type) {
	case MonsterType::MT_Movable:
		velocity.y = 0.f;
		velocity.x = movableMonsterXSpeed;
		break;
	default:
		velocity = 0.f;
		break;
	}
	bIsActive = true;
	spriteToDraw = 0;
	return IsActive();
}
