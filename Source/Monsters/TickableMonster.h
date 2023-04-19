#pragma once
#include "Monster.h"
#include "Tickable.h"

class TickableMonster : public Monster<RectangleShape>, public virtual Tickable
{
public:
	TickableMonster(DragonJumpFramework& _framework, const Vector2Df& pos, MonsterType _type);
	[[nodiscard]] static float GetMaxTickDeltaTime() { return 0.5f; };
	[[nodiscard]] virtual Vector2Df GetPosition() const override;
	virtual void ReceiveCollision(CollidableBase& other);
	[[nodiscard]] virtual bool IsActive() override;

	[[nodiscard]] virtual bool CanBeSteppedOn() const override ;
	virtual void OnJumpFrom(PlayerDoodle& other);
	virtual void ReceiveTick(float deltaTime);
	//[[nodiscard]] virtual bool Init(MonsterType _type);
	[[nodiscard]] virtual bool Reactivate(const Vector2Df& pos) override;

protected:
	[[maybe_unused]] virtual bool DrawIfActive_Internal() override;
	std::vector<SpriteInfo> spriteInfos;
	int spriteToDraw;
	float knockoutAnimationDuration;
	float dyingAnimationDuration;
	float dyingAnimationTime;
	float idleAnimationTime;
	float idleAnimationPeriod;

	static const float movableMonsterXSpeed;
};

