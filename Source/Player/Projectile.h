// by Dmitry Kolontay

#pragma once

#include "Tickable.h"
#include "Collidable.h"

struct SpriteInfo;

class Projectile : public Tickable, public Collidable<CircleShape>
{
public:
	Projectile(DragonJumpFramework& _framework, const Vector2Df& targetPos);
	virtual bool Reactivate(const Vector2Df& target) & override;
	virtual bool IsActive() override;

	static float GetMaxTickDeltaTimeStatic() 
		{ return 0.02f; };
	virtual float GetMaxTickDeltaTime() const override final 
		{ return GetMaxTickDeltaTimeStatic(); };

	virtual void ReceiveTick(float deltaTime) & override;

	virtual CollisionChannel GetCollisionChannel() const override 
		{ return CollisionChannel::CC_Projectile; };
	virtual bool GetCollisionResponse(CollisionChannel channel) const override 
		{ return channel == CollisionChannel::CC_Monster; };
	virtual void ReceiveCollision(CollidableBase& other) override;
	
protected:
	virtual bool DrawIfActive_Internal() override;
	SpriteInfo sprite;

	static constexpr float projectileSpeed = 500.f;
};
