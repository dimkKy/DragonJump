// by Dmitry Kolontay

#include "AnimatedPlatform.h"
#include "SpriteLoader.h"
#include "PlayerDoodle.h"
#include <cassert>
#include <string>

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
		spriteName = SpritePaths::defaultPlatform;
		break;
	default:
		assert(false && "AnimatedPlatform : wrong platfrom type");
		bIsActive = false;
		return;
	}
	if (framework.GetSpriteInfo(spriteName, defaultSprite)) {
		defaultSprite.offset *= 0.5f;
		collisionInfo.halfSize = defaultSprite.offset;
		collisionInfo.halfSize.x *= 0.9f;
	}
	else {
		assert(false && "AnimatedPlatform : defaultSprite init failed");
		bIsActive = false;
		return;
	}
	if (type == PlatformType::PT_Trampoline) {
		spriteName = SpritePaths::trampolinePlatform;
	}
	if (!framework.GetNumberedSprites(spriteName, animSprites)) {
		assert(false && "AnimatedPlatform : Trampoline sprites init failed");
		collisionInfo.halfSize = 0.f;
		bIsActive = false;
		return;
	}
	for (auto& sprite : animSprites) {
		sprite.offset *= 0.5f;
		if (type == PlatformType::PT_Trampoline) {
			sprite.offset.y += (defaultSprite.offset.y + sprite.offset.y * 0.85f);
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

bool AnimatedPlatform::Reactivate(const Vector2Df& pos)
{
	if (Platform::Reactivate(pos)) {
		velocity = 0.f;
		timeFromAnimStart = -1.f;
	}
	assert(bIsActive && "AnimatedPlatform reported faied reactivation");
	return bIsActive;
}

void AnimatedPlatform::ReceiveTick(float deltaTime)
{
	if (timeFromAnimStart >= 0.f && timeFromAnimStart < animationDuration) {
		timeFromAnimStart += deltaTime;
	}
	switch (type) {
	case PlatformType::PT_Weak:
		if (timeFromAnimStart > 0.f) {
			position.y += velocity.y * deltaTime;
			velocity.y += fallingAcceleration * deltaTime;
		}
		return;
	case PlatformType::PT_SelfDestuct:
		if (timeFromAnimStart < 0.f && 
			(framework.GetPlayerDoodle()->GetPosition() - position).LengthSquared() < 
			collisionInfo.halfSize.LengthSquared() * selfDestuctTriggerDistMp) {
			timeFromAnimStart = 0.f;
		}
		break;
	default:
		break;
	}
}

bool AnimatedPlatform::CanBeSteppedOn() const
{
	assert(type != PlatformType::PT_COUNT && "AnimatedPlatform reported wrong platfrom type");
	return type != PlatformType::PT_Weak;
}

void AnimatedPlatform::ReceiveCollision(CollidableBase& other)
{
	if (type != PlatformType::PT_Weak) {
		return;
	}
	if (PlayerDoodle * doodle{ dynamic_cast<PlayerDoodle*>(&other) }) {
		if (timeFromAnimStart < 0.f && doodle->GetVelocity().y > 0.f && 
			doodle->GetPosition().y < position.y + collisionInfo.halfSize.y) {
			velocity.y = doodle->GetVelocity().y;
			timeFromAnimStart = 0.f;
		}
	}
}

void AnimatedPlatform::OnJumpFrom(PlayerDoodle& other)
{
	switch (type) {
	case PlatformType::PT_OneOff:
		bIsActive = false;
		break;
	case PlatformType::PT_Trampoline:
		DJLog("Trampoline : added impulse");
		other.AddImpulse({ 0.f, trampolineImpulse }, 0.f);
		timeFromAnimStart = 0.f;
		break;
	default:
		break;
	}	
}

bool AnimatedPlatform::DrawIfActive_Internal()
{
	if (position.y + defaultSprite.offset.y <= +0.f) {
		return false;
	}
	if (timeFromAnimStart >= animationDuration) {
		if (OnAnimEnded()) {
			return false;
		}
	}
	//WEAK PLATFORM INCORRECT DRAW
	int spriteToDraw{ timeFromAnimStart < 0.f ? -1 :
			static_cast<int>(timeFromAnimStart * animSprites.size() / animationDuration) };

	if (type == PlatformType::PT_Trampoline) {
		if (framework.IsOutOfSideBorder(position, defaultSprite.sprite, true) == 0) {
			defaultSprite.Draw(position);
		}
		animSprites.at(spriteToDraw < 0 ? 0 : spriteToDraw).Draw(position);
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

bool AnimatedPlatform::OnAnimEnded()
{
	if (type == PlatformType::PT_Trampoline) {
		timeFromAnimStart = -1.f;
		return false;
	}
	else {
		if (type != PlatformType::PT_Invisible && 
			type != PlatformType::PT_Weak) {
			bIsActive = false;
		}
		return true;
	}
}

