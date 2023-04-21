// by Dmitry Kolontay

#include "Ability.h"
#include "SpriteLoader.h"
#include <cassert>

Ability::Ability(DragonJumpFramework& _framework, AbilityType _type, const Vector2Df& position) :
	Drawable(_framework, position), type{_type}
{
	std::string path;
	switch (type) {
	case AbilityType::AT_Jet:
		path = SpritePaths::jetAbility;
		break;
	case AbilityType::AT_Shield:
		path = SpritePaths::jetAbility;
		break;
	default:
		bIsActive = false;
		type = AbilityType::AT_COUNT;
		break;
	}
	if (framework.GetSpriteInfo(path, sprite)) {
		collisionInfo.radiusSquared = sprite.offset.LengthSquared();
		sprite.offset *= 0.5f;
	}
	else {
		bIsActive = false;
		type = AbilityType::AT_COUNT;
	}
	assert(bIsActive && "Ability init unsuccessfull");
}

bool Ability::Reactivate(const Vector2Df& pos)
{
	position = pos;
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
