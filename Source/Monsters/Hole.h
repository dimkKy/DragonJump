// by Dmitry Kolontay

#pragma once

#include "Monster.h"

class Hole : public Monster<CircleShape>
{
public:
	Hole(DragonJumpFramework& _framework, const Vector2Df& _position);

	[[nodiscard]] virtual bool Reactivate(const Vector2Df& newPosition) & override;
	[[nodiscard]] virtual bool IsActive() override;
	
	[[nodiscard]] virtual bool CanBeSteppedOn() const override final 
		{ return false; } ;
protected:
	[[maybe_unused]] virtual bool DrawIfActive_Internal() override;
	SpriteInfo sprite;
};

