// by Dmitry Kolontay

#pragma once

#include "Collidable.h"

class PlayerDoodle;

enum class AbilityType {
	AT_Jet,
	AT_Shield,
	AT_COUNT
};

class Ability : public Collidable<CircleShape>
{
public:
	Ability(DragonJumpFramework& _framework, const Vector2Df& pos, AbilityType _type);
	AbilityType GetAbilityType() const 
		{ return type; };

	static void OnAbilityTick(PlayerDoodle& doodle, AbilityType _type, float deltaTime);

	static float GetAbilityDuration(AbilityType _type);
	float GetAbilityDuration() const
		{ return GetAbilityDuration(type);};
	virtual bool IsActive() override;
	virtual bool Reactivate(const Vector2Df& pos) & override;
	
	virtual CollisionChannel GetCollisionChannel() const override 
		{ return CC::Pickup; };
	virtual bool GetCollisionResponse(CollisionChannel channel) const override 
		{ return channel == CC::Player; };
	void ReceiveCollision(Collidable& other) {};

	static constexpr float jetAnimDuration{ 0.75f };
protected:
	virtual bool DrawIfActive_Internal() override;
	SpriteInfo sprite;
	AbilityType type;

	static constexpr float jetForce{ -55000.f };
};

