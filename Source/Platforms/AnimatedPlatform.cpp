#include "AnimatedPlatform.h"
#include "SpriteLoader.h"
#include "PlayerDoodle.h"
#include <cassert>
#include <string>

const float AnimatedPlatform::fallingAcceleration = 200.f;
const float AnimatedPlatform::trampolineImpulse = -150.f;
const float AnimatedPlatform::defaultAnimDuration = 0.85f;
const float AnimatedPlatform::selfDestuctAnimDuration = 1.75f;
const float AnimatedPlatform::invisibleAnimDuration = 2.f;
const float AnimatedPlatform::selfDestuctTriggerDistMp = 4.f;
const float AnimatedPlatform::invisibleAnimTriggerDist = 0.2f;

AnimatedPlatform::AnimatedPlatform(DragonJumpFramework& _framework, const Vector2Df& position, PlatformType _type) :
	Drawable(_framework, position), TickablePlatform(_framework, _type), 
	animationDuration{ defaultAnimDuration }, timeFromAnimStart{ -1.f }
{
	std::string spriteName;
	switch (_type) {
	case PlatformType::PT_OneOff:
		spriteName = SpritePaths::oneOffPlatform;
		break;
	case PlatformType::PT_SelfDestuct:
		spriteName = SpritePaths::selfDestructPlatform;
		animationDuration = selfDestuctAnimDuration;
		break;
	case PlatformType::PT_Invisible:
		spriteName = SpritePaths::invisiblePlatform;
		animationDuration = invisibleAnimDuration;
		break;
	case PlatformType::PT_Weak:
		spriteName = SpritePaths::weakPlatform;
		break;
	case PlatformType::PT_Trampoline:
		spriteName = SpritePaths::trampolinePlatform;
		break;
	default:
		assert(false && "AnimatedPlatform : wrong platfrom type");
		bIsActive = false;
		return;
	}
	if (framework.GetSpriteInfo(spriteName, defaultSprite)) {
		collisionInfo.size = defaultSprite.offset;
		defaultSprite.offset * 0.5f;
	}
	else {
		assert(false && "AnimatedPlatform : defaultSprite init failed");
		bIsActive = false;
		return;
	}
	if (!framework.GetNumberedSprites(spriteName, animSprites) && 
		type == PlatformType::PT_Trampoline) {
		assert(false && "AnimatedPlatform : Trampoline sprites init failed");
		collisionInfo.size = 0.f;
		bIsActive = false;
		return;
	}
	for (auto& sprite : animSprites) {
		sprite.offset *= 0.5f;
		if (type == PlatformType::PT_Trampoline) {
			sprite.offset.y -= (defaultSprite.offset.y + sprite.offset.y * 0.9f);
		}
	}
}

bool AnimatedPlatform::IsActive()
{
	if (!Collidable::IsActive()) {
		return false;
	}
	if (type == PlatformType::PT_Trampoline) {
		bIsActive = framework.IsOutOfSideBorder(position,
			defaultSprite.sprite) == 0 &&
			position.y - animSprites.at(0).offset.y < framework.GetSize().y;
	}
	else {
		bIsActive = framework.IsInScreenArea(position, defaultSprite.sprite);
	}
	return bIsActive;
}

bool AnimatedPlatform::Reactivate(const Vector2Df& position)
{
	if (Platform::Reactivate(position)) {
		currentVelocity = 0.f;
		timeFromAnimStart = -1.f;
	}
	assert(bIsActive && "AnimatedPlatform reported faied reactivation");
	return bIsActive;
}

void AnimatedPlatform::ReceiveTick(float deltaTime)
{
	if (timeFromAnimStart >= 0.f && timeFromAnimStart < animationDuration)
		timeFromAnimStart += deltaTime;

	switch (type) {
	case PlatformType::PT_Weak:
		if (timeFromAnimStart > 0.f) {
			position.y += currentVelocity.y * deltaTime;
			currentVelocity.y += fallingAcceleration * deltaTime;
		}
		return;
	case PlatformType::PT_SelfDestuct:
		if (timeFromAnimStart < 0.f && 
			(framework.GetPlayerDoodle()->GetPosition() - position).LengthSquared() < 
			collisionInfo.size.LengthSquared() * selfDestuctTriggerDistMp) {
			timeFromAnimStart = 0.f;
		}
		break;
	default:
		break;
	}
	if (timeFromAnimStart > animationDuration) {
		OnAnimEnded();
	}
}

bool AnimatedPlatform::CanBeSteppedOn() const
{
	assert(type != PlatformType::PT_COUNT && "AnimatedPlatform reported wrong platfrom type");
	return type != PlatformType::PT_Weak;
}

void AnimatedPlatform::ReceiveCollision(CollidableBase& other)
{
	if (PlayerDoodle * doodle{ dynamic_cast<PlayerDoodle*>(&other) }) {
		if (type == PlatformType::PT_Weak && currentVelocity.y == 0.f && 
			timeFromAnimStart < 0.f && doodle->GetCurrentVelocity().y > 0.f && 
			doodle->GetPosition().y < position.y - collisionInfo.size.y) {
			currentVelocity.y = doodle->GetCurrentVelocity().y;
			timeFromAnimStart = 0.f;
		}
	}
}

void AnimatedPlatform::OnJumpFrom(PlayerDoodle& other)
{
	if (type == PlatformType::PT_Trampoline) {
		other.AddImpulse({ 0.f, trampolineImpulse }, 0.f);
		timeFromAnimStart = 0.f;
	}	
}

bool AnimatedPlatform::DrawIfActive_Internal()
{
	if (position.y + defaultSprite.offset.y <= +0.f) {
		return false;
	}
	int spriteToDraw{ timeFromAnimStart >= 0.f ? -1 : static_cast<int>(
		timeFromAnimStart / static_cast<float>(animationDuration / animSprites.size())) };
	if (type == PlatformType::PT_Trampoline) {
		if (framework.IsOutOfSideBorder(position, defaultSprite.sprite, true) == 0) {
			defaultSprite.Draw(position);
		}
		animSprites.at(spriteToDraw + 1).Draw(position);
		return true;
	}
	else {
		if (spriteToDraw == -1) {
			defaultSprite.Draw(position);
			if (type == PlatformType::PT_Invisible && timeFromAnimStart < 0.f && 
				position.y > framework.GetSize().y * invisibleAnimTriggerDist) {
				timeFromAnimStart = 0.f;
			}
		}
		else {
			animSprites.at(spriteToDraw).Draw(position);
		}
	}
	return true;
}

void AnimatedPlatform::OnAnimEnded()
{
	if (type == PlatformType::PT_Trampoline) {
		timeFromAnimStart = -1.f;
	}
	else {
		bIsActive = false;
	}
}

