// by Dmitry Kolontay

#pragma once

#include "Monster.h"
#include "Tickable.h"

class TickableMonster : public Monster<RectangleShape>, public Tickable
{
public:
	TickableMonster(DragonJumpFramework& _f, const Vector2Df& pos, MonsterType _type);
	[[nodiscard]] static float GetMaxTickDeltaTime() { return 0.5f; };
	[[nodiscard]] virtual Vector2Df GetPosition() const override;
	virtual void ReceiveCollision(CollidableBase& other);
	[[nodiscard]] virtual bool IsActive() override;

	[[nodiscard]] virtual bool CanBeSteppedOn() const override ;
	virtual void OnJumpFrom(PlayerDoodle& other);
	virtual void ReceiveTick(float deltaTime);
	[[nodiscard]] virtual bool Reactivate(const Vector2Df& pos) override;

protected:
	[[maybe_unused]] virtual bool DrawIfActive_Internal() override;
	[[nodiscard]] float GetAnimOffsetY() const;
	std::vector<SpriteInfo> spriteInfos;
	int spriteToDraw;
	
	float dyingAnimationTime;
	float idleAnimationTime;

	static constexpr float knockoutAnimationDuration = 3.f;
	static constexpr float idleAnimationPeriod = 2.f;
	static constexpr float dyingAnimationDuration = 0.25f;
	static constexpr float dyingVerticalSpeed = 150.f;
	static constexpr float idleAnimAmplitudeMp = 0.35f;
	static constexpr float movableMonsterXSpeed = 100.f;
};
