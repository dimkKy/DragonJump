//extern template class Collidable<CircleShape>;
//extern template class Collidable<RectangleShape>;

/*class Collider {
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
	std::span<std::shared_ptr<SCollidable>> second, bool checkIsActive = true, bool checkChannels = false);*
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
};*/

