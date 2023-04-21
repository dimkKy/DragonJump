// by Dmitry Kolontay

#include "Collider.h"
#include "Collidable.h"

template<>
bool Collider::DetectCollision(const Collidable<CircleShape>& first, const Collidable<CircleShape>& second) {
	return (first.position - second.position).LengthSquared() <=
		first.collisionInfo.radiusSquared + second.collisionInfo.radiusSquared +
		2 * sqrtf(first.collisionInfo.radiusSquared * second.collisionInfo.radiusSquared);
}

template<>
bool Collider::DetectCollision(const Collidable<RectangleShape>& first, const Collidable<RectangleShape>& second) {
	return (std::abs(first.position.x - second.position.x) <= first.collisionInfo.halfSize.x + second.collisionInfo.halfSize.x &&
		std::abs(first.position.y - second.position.y) <= first.collisionInfo.halfSize.y + second.collisionInfo.halfSize.y);
}

template<>
bool Collider::DetectCollision(const Collidable<CircleShape>& first, const Collidable<RectangleShape>& second) {
	Vector2Df rectHalfSize{ second.collisionInfo.halfSize * 0.5f };
	Vector2Df closestToSpherePoint{
		std::clamp(first.position.x, second.position.x - rectHalfSize.x, second.position.x + rectHalfSize.x),
		std::clamp(first.position.y, second.position.y - rectHalfSize.y, second.position.y + rectHalfSize.y) };
	return ((closestToSpherePoint - first.position).LengthSquared() <= first.collisionInfo.radiusSquared) ||
		first.position.IsPointInsideRectangle(second.position - rectHalfSize, second.position + rectHalfSize);
}

template<> bool Collider::DetectCollision(const Collidable<RectangleShape>& first, const Collidable<CircleShape>& second) {
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
