// by Dmitry Kolontay

#include "Ability.h"
#include "SpriteLoader.h"
#include "PlayerDoodle.h"
#include <cassert>

Ability::Ability(DragonJumpFramework& _framework, const Vector2Df& _position, AbilityType _type) :
	Drawable(_framework, _position), type{_type}
{
	std::string path;
	switch (type) {
	case AbilityType::AT_Jet:
		path = SpritePaths::jetAbility;
		break;
	case AbilityType::AT_Shield:
		path = SpritePaths::shieldAbility;
		break;
	default:
		bIsActive = false;
		type = AbilityType::AT_COUNT;
		break;
	}
	if (framework.GetSpriteInfo(path, sprite)) {
		collisionInfo.radiusSquared = sprite.offset.LengthSquared();
		sprite.offset *= 0.5f;
		position.y -= sprite.offset.y * 1.1f;
	}
	else {
		bIsActive = false;
		type = AbilityType::AT_COUNT;
	}
	assert(bIsActive && "Ability init unsuccessfull");
}

bool Ability::Reactivate(const Vector2Df& pos) &
{
	position.x = pos.x;
	//?!
	position.y = pos.y - sprite.offset.y * 1.1f;
	bIsActive = true;
	return IsActive();
}

bool Ability::DrawIfActive_Internal()
{
	if (position.y + sprite.offset.y > +0.f) {
		sprite.Draw(position);
		return true;
	}
	else {
		return false;
	}	
}

bool Ability::IsActive()
{
	if (Collidable::IsActive()) {
		bIsActive = framework.IsInScreenArea(position, 
			framework.GetSpriteSize(sprite.sprite));
	}
	return bIsActive;
}

void Ability::OnAbilityTick(PlayerDoodle& doodle, AbilityType _type, float deltaTime)
{
	if (_type == AbilityType::AT_Jet) {
		doodle.AddImpulse({0.f, jetForce }, deltaTime);
	}
}

float Ability::GetAbilityDuration(AbilityType _type)
{
	switch (_type) {
	case AbilityType::AT_Jet:
		return 5.f;
	case AbilityType::AT_Shield:
		return 20.f;
	default:
		assert(_type != AbilityType::AT_COUNT && "GetAbilityDuration reported wrong type");
		return 0.f;
	}
}
