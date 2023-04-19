#pragma once
#include "Collidable.h"

class SteppableOnBase 
{
public:
	[[nodiscard]] virtual bool CanBeSteppedOn() const 
		{ return true; };
	[[nodiscard]] virtual bool GetStandingPointY(const float& inX, float& outY, bool modeRelative = false) const = 0;
	virtual void OnJumpFrom(PlayerDoodle& other) = 0;
protected:
	SteppableOnBase() {};
};

template<class TShape>
class SteppableOn : public SteppableOnBase, public Collidable<TShape>
{
public:
	//[[nodiscard]] virtual bool CanBeSteppedOn() const { return true; };
	[[maybe_unused]] virtual bool GetStandingPointY(const float& inX, float& outY, bool modeRelative = false) const override;
	//virtual void OnJumpFrom(PlayerDoodle& other) = 0;
protected:
	template<class...Args>
	SteppableOn(const Args&...args) : 
		Collidable<TShape>(std::forward<Args>(args)...) {};
};

extern template class SteppableOn<RectangleShape>;

template <> bool SteppableOn<RectangleShape>::GetStandingPointY(const float& inX, float& outY, bool modeRelative) const {
	float xDist{ modeRelative ? abs(inX) : abs(inX - position.x) };
	if (xDist > collisionInfo.size.x * 0.5f) {
		return false;
	}
	else {
		outY = modeRelative ? -collisionInfo.size.y * -0.5f :
			position.y - collisionInfo.size.y * -0.5f;
		return true;
	}
}

template <> bool SteppableOn<CircleShape>::GetStandingPointY(const float& inX, float& outY, bool modeRelative) const {
	float xDistSquared{ modeRelative ? inX * inX : std::powf(inX - position.x, 2) };
	if (xDistSquared > collisionInfo.radiusSquared) {
		return false;
	}
	else {
		outY = modeRelative ? -sqrtf(collisionInfo.radiusSquared - xDistSquared) :
			position.y - sqrtf(collisionInfo.radiusSquared - xDistSquared);
		return true;
	}
}

