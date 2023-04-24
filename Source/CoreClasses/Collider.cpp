// by Dmitry Kolontay

#include "Collider.h"
#include "Collidable.h"

template<>
bool Collider::DetectCollision(const Collidable<CircleShape>& first, const Collidable<CircleShape>& second) {
	return (first.GetPosition() - second.GetPosition()).LengthSquared() <=
		first.collisionInfo.radiusSquared + second.collisionInfo.radiusSquared +
		2 * sqrtf(first.collisionInfo.radiusSquared * second.collisionInfo.radiusSquared);
}

template<>
bool Collider::DetectCollision(const Collidable<RectangleShape>& first, const Collidable<RectangleShape>& second) {
	Vector2Df fPos{ first.GetPosition() };
	Vector2Df sPos{ second.GetPosition() };
	return (std::abs(fPos.x - sPos.x) <= first.collisionInfo.halfSize.x + second.collisionInfo.halfSize.x &&
		std::abs(fPos.y - sPos.y) <= first.collisionInfo.halfSize.y + second.collisionInfo.halfSize.y);
}

template<>
bool Collider::DetectCollision(const Collidable<CircleShape>& first, const Collidable<RectangleShape>& second) {
	Vector2Df fPos{ first.GetPosition() };
	Vector2Df sPos{ second.GetPosition() };
	Vector2Df closestToSpherePoint{
		std::clamp(fPos.x, sPos.x - second.collisionInfo.halfSize.x, sPos.x + second.collisionInfo.halfSize.x),
		std::clamp(fPos.y, sPos.y - second.collisionInfo.halfSize.y, sPos.y + second.collisionInfo.halfSize.y) };
	return ((closestToSpherePoint - fPos).LengthSquared() <= first.collisionInfo.radiusSquared) ||
		fPos.IsPointInsideRectangle(sPos - second.collisionInfo.halfSize, sPos + second.collisionInfo.halfSize);
}

template<> 
bool Collider::DetectCollision(const Collidable<RectangleShape>& first, const Collidable<CircleShape>& second) {
	{ return DetectCollision(second, first); };
}

void Collider::DispatchCollision(CollidableBase& first, CollidableBase& second, bool checkChannels)
{
	if (!checkChannels || first.GetCollisionResponse(second.GetCollisionChannel())) {
		first.ReceiveCollision(second);
	}
	if (!checkChannels || second.GetCollisionResponse(first.GetCollisionChannel())) {
		second.ReceiveCollision(first);
	}
}
