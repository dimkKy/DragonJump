extern template class Collidable<CircleShape>;
extern template class Collidable<RectangleShape>;

class Collider {
public:
	//template<class FCollidable, class SCollidable>
	//FCollidable& first, SCollidable& second
	//Collidable<FShape>& first, Collidable<SShape>& second
	//template <template<class> typename FCollidable, class FShape, template<class> typename FCollidable, class FShape>
	//template<class FShape, class SShape>
	template<class FCollidable, class SCollidable>
	[[maybe_unused]] static bool DetectDispatchCollision(FCollidable& first, SCollidable& second,
		bool dispatch = true, bool checkIsActive = true, bool checkChannels = false) {
		if (checkIsActive && (!first.bIsActive || !second.bIsActive)) {
			return false;
		}	
		if (checkChannels && !first.GetCollisionResponse(second.GetCollisionChannel()) &&
			!second.GetCollisionResponse(first.GetCollisionChannel())) {
			return false;
		}
		if (DetectCollision(first, second)) {
			if (dispatch) {
				DispatchCollision(first, second, checkChannels);
			}	
			return true;
		}
		else {
			return false;
		}
	}
	/*template<class FCollidable, class SCollidable>
	[[nodiscard]] static bool DetectCollisions(std::span<std::shared_ptr<FCollidable>> first,
	std::span<std::shared_ptr<SCollidable>> second, bool checkIsActive = true, bool checkChannels = false);*/
protected:
	//template<class FCollidable, class SCollidable>
	//template<class FShape, class SShape>
	static void DispatchCollision(CollidableBase& first, CollidableBase& second, bool checkChannels) {
		if (!checkChannels || first.GetCollisionResponse(second.GetCollisionChannel())) {
			first.ReceiveCollision(second);
		}	
		if (!checkChannels || second.GetCollisionResponse(first.GetCollisionChannel())) {
			second.ReceiveCollision(first);
		}	
	}
	template<class FShape, class SShape>
	[[nodiscard]] static bool DetectCollision(const Collidable<FShape>& first, const Collidable<SShape>& second);
};

template<> 
bool Collider::DetectCollision(const Collidable<CircleShape>& first, const Collidable<CircleShape>& second) {
	return (first.position - second.position).LengthSquared() <=
		first.collisionInfo.radiusSquared + second.collisionInfo.radiusSquared +
		2 * sqrtf(first.collisionInfo.radiusSquared * second.collisionInfo.radiusSquared);
}

template<> 
bool Collider::DetectCollision( const Collidable<RectangleShape>& first, const Collidable<RectangleShape>& second) {
	return (std::abs(first.position.x - second.position.x) <= first.collisionInfo.size.x + second.collisionInfo.size.x &&
		std::abs(first.position.y - second.position.y) <= first.collisionInfo.size.y + second.collisionInfo.size.y);
}

template<> 
bool Collider::DetectCollision(const Collidable<CircleShape>& first, const Collidable<RectangleShape>& second) {
	Vector2Df rectHalfSize{ second.collisionInfo.size * 0.5f };
	Vector2Df closestToSpherePoint{
		std::clamp(first.position.x, second.position.x - rectHalfSize.x, second.position.x + rectHalfSize.x),
		std::clamp(first.position.y, second.position.y - rectHalfSize.y, second.position.y + rectHalfSize.y) };
	return ((closestToSpherePoint - first.position).LengthSquared() <= first.collisionInfo.radiusSquared) ||
		first.position.IsPointInsideRectangle(second.position - rectHalfSize, second.position + rectHalfSize);
}

template<> bool Collider::DetectCollision(const Collidable<RectangleShape>& first, const Collidable<CircleShape>& second) {
	{ return DetectCollision(second, first); };
}