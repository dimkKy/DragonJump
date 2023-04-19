#pragma once
#include "Collidable.h"

enum class AbilityType {
	AT_Jet,
	AT_Shield,
	AT_COUNT
};

class Ability : public Collidable<CircleShape>
{
public:
	Ability(DragonJumpFramework& _framework, AbilityType _type, const Vector2Df& position);
	AbilityType GetAbilityType() const 
		{ return type; };
	static float GetAbilityDuration(AbilityType _type);

	virtual bool IsActive() override;
	virtual bool Reactivate(const Vector2Df& position) override;
	
	virtual CollisionChannel GetCollisionChannel() const override 
		{ return CollisionChannel::CC_Pikup; };
	virtual bool GetCollisionResponse(CollisionChannel channel) const override 
		{ return channel == CollisionChannel::CC_Player; };
	void ReceiveCollision(Collidable& other) {};
protected:
	virtual bool DrawIfActive_Internal() override;
	SpriteInfo sprite;
	AbilityType type;
};

