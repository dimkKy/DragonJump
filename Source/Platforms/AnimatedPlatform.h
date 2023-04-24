// by Dmitry Kolontay

#pragma once

#include "TickablePlatform.h"

class AnimatedPlatform : public TickablePlatform
{
public:
	AnimatedPlatform(DragonJumpFramework& _framework, const Vector2Df& position, PlatformType _type);

	virtual bool IsActive() override;
	virtual bool Reactivate(const Vector2Df& pos) override;

	virtual void ReceiveCollision(CollidableBase& other) override;
	[[nodiscard]] virtual bool CanBeSteppedOn() const override;
	virtual void OnJumpFrom(PlayerDoodle& other) override;

	virtual void ReceiveTick(float deltaTime) override;
protected:
	//throws out of bounds
	virtual bool DrawIfActive_Internal() override;
	bool OnAnimEnded();
	std::vector<SpriteInfo> animSprites;
	float animationDuration;
	float timeFromAnimStart;

	static constexpr float fallingAcceleration = 200.f;
	static constexpr float trampolineImpulse = -125.f;
	static constexpr float defaultAnimDuration = 0.75f;
	static constexpr float selfDestuctAnimDuration = 1.75f;
	static constexpr float invisibleAnimDuration = 2.f;
	static constexpr float selfDestuctTriggerDistMp = 4.f;
	static constexpr float invisibleAnimTriggerDist = 0.2f;
};

