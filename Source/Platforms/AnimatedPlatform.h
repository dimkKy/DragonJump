#pragma once
#include "TickablePlatform.h"


class AnimatedPlatform : public TickablePlatform
{
public:
	AnimatedPlatform(DragonJumpFramework& _framework, const Vector2Df& position, PlatformType _type);

	virtual bool IsActive() override;
	virtual bool Reactivate(const Vector2Df& position) override;

	virtual void ReceiveCollision(CollidableBase& other) override;
	[[nodiscard]] virtual bool CanBeSteppedOn() const override;
	virtual void OnJumpFrom(PlayerDoodle& other) override;

	virtual void ReceiveTick(float deltaTime) override;
protected:
	//throws out of bounds
	virtual bool DrawIfActive_Internal() override;
	void OnAnimEnded();
	std::vector<SpriteInfo> animSprites;
	float animationDuration;
	float timeFromAnimStart;

	static const float fallingAcceleration;
	static const float trampolineImpulse;
	static const float defaultAnimDuration;
	static const float selfDestuctAnimDuration;
	static const float invisibleAnimDuration;
	static const float selfDestuctTriggerDistMp;
	static const float invisibleAnimTriggerDist;
};

