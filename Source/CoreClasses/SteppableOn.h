// by Dmitry Kolontay

#pragma once

#include "Collidable.h"

class SteppableOnBase 
{
public:
	[[nodiscard]] virtual bool CanBeSteppedOn() const 
		{ return true; };
	[[nodiscard]] virtual float GetStandingPointY(
		const float& inX, bool modeRelative = false) const = 0;
	virtual void OnJumpFrom(PlayerDoodle& other) {};
protected:
	SteppableOnBase() {};
};

template<class TShape>
class SteppableOn : public SteppableOnBase, public Collidable<TShape>
{
public:
	[[nodiscard]] virtual float GetStandingPointY(
		const float& inX, bool modeRelative = false) const override;
protected:
	template<class...Args>
	SteppableOn(const Args&...args) : 
		Collidable<TShape>(std::forward<Args>(args)...) {};
};

extern template class SteppableOn<RectangleShape>;