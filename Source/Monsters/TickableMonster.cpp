#include "TIckableMonster.h"
#include "Projectile.h"
#include "SpriteLoader.h"
#include <cassert>
#include <numbers>

const float TickableMonster::movableMonsterXSpeed = 125.f;

TickableMonster::TickableMonster(DragonJumpFramework& _framework, const Vector2Df& position, MonsterType _type) :
	Drawable(_framework, position), Monster(_type), spriteToDraw{ 0 }, knockoutAnimationDuration { 3.f },
	dyingAnimationDuration{ 0.25f }, dyingAnimationTime{ -1.f }, idleAnimationTime{ 0.f }, idleAnimationPeriod{ 2.f }
{
	std::string path;
	int desiredSpriteNumber{ 2 };
	switch (type) {
	case MonsterType::MT_Static:
		path = SpritePaths::staticMonster;
		break;
	case MonsterType::MT_Movable:
		path = SpritePaths::movableMonster;
		break;
	default:
		break;
	}
	framework.GetNumberedSprites(path, spriteInfos);
	if (spriteInfos.size() == desiredSpriteNumber) {
		for (auto& info : spriteInfos) {
			info.offset *= 0.5f;
		}
	}
	else {
		DJLog("TickableMonster reported incorrect sprites");
		bIsActive = false;
		type = MonsterType::MT_COUNT;
	}
}

Vector2Df TickableMonster::GetPosition() const
{
	return { position.x, position.y + spriteInfos.at(0).offset.y * 
		0.4f * sinf(idleAnimationTime / idleAnimationPeriod * std::numbers::pi_v<float>) };
}

bool TickableMonster::DrawIfActive_Internal()
{	
	if (position.y + spriteInfos.at(0).offset.y > +0.f) {
		//can be done better
		/*if (dyingAnimationTime > dyingAnimationDuration && currentVelocity.y == 0.f) {
			return false;
		}*/
			
		float animOffsetY{ 0.f };
		if (dyingAnimationTime < 0.f) {
			animOffsetY = spriteInfos.at(0).offset.y * 0.4f *
				sinf(idleAnimationTime / idleAnimationPeriod * std::numbers::pi_v<float>);
		}
		spriteInfos.at(spriteToDraw).Draw(position.x, position.y + animOffsetY);
		return true;
	}
	else
		return false;
	
}


void TickableMonster::ReceiveCollision(CollidableBase& other)
{
	if (Projectile * projectile{ dynamic_cast<Projectile*>(&other) }) {
		if (dyingAnimationTime < 0.f) {
			dyingAnimationTime = 0.f;
			currentVelocity.x = 0.f;
			currentVelocity.y = 0.f;
			framework.IncreaseMonstersKilledCounter();
		}	
	}
}

bool TickableMonster::IsActive()
{
	if (Collidable::IsActive()) {
		if (dyingAnimationTime > dyingAnimationDuration && currentVelocity.y == 0.f) {
			bIsActive = false;
			return bIsActive;
		}
		bIsActive = framework.IsInScreenArea(position, 
			framework.GetSpriteSize(spriteInfos.at(spriteToDraw).sprite));
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
		currentVelocity.x = 0.f;
		currentVelocity.y = 150.f;
		framework.IncreaseMonstersKilledCounter();
	}
}

void TickableMonster::ReceiveTick(float deltaTime)
{
	if (dyingAnimationTime >= 0.f) {
		dyingAnimationTime += deltaTime;
		if (currentVelocity.y > 0.f) {
			if (dyingAnimationTime >= knockoutAnimationDuration) {
				dyingAnimationTime = std::fmodf(dyingAnimationTime, knockoutAnimationDuration);
			}
			position.y += currentVelocity.y * deltaTime;
		}
		else {
			if (dyingAnimationTime > dyingAnimationDuration) {
				bIsActive = false;
			}
		}
		return;
	}
	position += currentVelocity * deltaTime;
	idleAnimationTime = std::fmodf(idleAnimationTime + deltaTime, idleAnimationPeriod);
	if (currentVelocity.x) {
		//REDO
		/*if (position.x + defaultSpriteInfo.offset.x > framework.GetSize().x && currentVelocity.x > 0.f) {
			position.x = framework.GetSize().x - defaultSpriteInfo.offset.x;
			currentVelocity.x *= -1;
		}
		else {
			if (position.x - defaultSpriteInfo.offset.x < 0.f && currentVelocity.x < 0.f) {
				position.x = defaultSpriteInfo.offset.x;
				currentVelocity.x *= -1;
			}
		}*/
	}
}

/*bool TickableMonster::Init(MonsterType _type)
{
	if (type == MonsterType::MT_COUNT) {
		std::shared_ptr<Sprite> tempSprite;
		Vector2D tempVec;
		switch (_type) {
		case MonsterType::MT_Static:
			if (framework.GetSprite(SpritePaths::staticMonsterDying, tempSprite)) {
				getSpriteSize(tempSprite.get(), tempVec.x, tempVec.y);
				dyingSpriteInfo.sprite = tempSprite;
				dyingSpriteInfo.offset = static_cast<Vector2Df>(tempVec) * 0.5f;
			}
			else {
				DJLog("aborting staticMonster init");
				return false;
			}
			break;
		case MonsterType::MT_Movable:
			if (framework.GetSprite(SpritePaths::movableMonsterRight, tempSprite)) {
				getSpriteSize(tempSprite.get(), tempVec.x, tempVec.y);
				dyingSpriteInfo.sprite = tempSprite;
				dyingSpriteInfo.offset = static_cast<Vector2Df>(tempVec) * 0.5f;
				currentVelocity.x = -125.f;
			}
			else {
				DJLog("aborting staticMonster init");
				return false;
			}
			break;
		default:
			assert(false && "TickableMonster reported wrong platfrom type");
			return false;
			break;
		}
		type = _type;
		return true;
	}
	return false;
}*/

bool TickableMonster::Reactivate(const Vector2Df& pos)
{
	if (type == MonsterType::MT_COUNT) {
		bIsActive = false;
		return false;
	}
	dyingAnimationTime = -1.f;
	position = pos;
	switch (type)
	{
	case MonsterType::MT_Movable:
		currentVelocity.y = 0.f;
		currentVelocity.x = movableMonsterXSpeed;
		break;
	default:
		currentVelocity = 0.f;
		break;
	}
	bIsActive = true;
	return IsActive();
}