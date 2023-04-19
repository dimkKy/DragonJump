#pragma once
#include "Drawable.h"

class Tickable : public virtual Drawable
{
public:
	//dispatcher is responsible for checking isActive
	virtual void ReceiveTick(float deltaTime) = 0;
	[[nodiscard]] virtual Vector2Df GetCurrentVelocity() const 
		{ return currentVelocity; };
	[[nodiscard]] virtual float GetMaxTickDeltaTime() const 
		{ return 0.f; };
protected:
	template<class...Args>
	Tickable(const Args&...args) : 
		currentVelocity{ args... } {};
	Vector2Df currentVelocity;
};

