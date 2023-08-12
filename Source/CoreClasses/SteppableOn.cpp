// by Dmitry Kolontay

#include "SteppableOn.h"

template <> float SteppableOn<RectangleShape>::GetStandingPointY(
	const float& inX, bool modeRelative) const {
	return modeRelative ? -collisionInfo.halfSize.y :
		position.y - collisionInfo.halfSize.y;
}

template <> float SteppableOn<CircleShape>::GetStandingPointY(
	const float& inX, bool modeRelative) const {
	float xDistSqrd{ modeRelative ? inX * inX : std::powf(inX - position.x, 2.f) };
	return modeRelative ? -sqrtf(collisionInfo.radiusSquared - xDistSqrd) :
		position.y - sqrtf(collisionInfo.radiusSquared - xDistSqrd);
}