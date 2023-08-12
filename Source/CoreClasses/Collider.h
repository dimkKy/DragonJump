// by Dmitry Kolontay

#pragma once
#include "Collidable.h"

class Collider {
public:
	[[maybe_unused]] static bool DetectDispatchCollision(AnyCollidable auto& first, AnyCollidable auto& second,
		bool dispatch = true, bool checkIsActive = true, bool checkChannels = false) {
		if (checkIsActive && (!first.bIsActive || !second.bIsActive)) {
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
	template<AnyCollidable SCollidable>
	static void DispatchCollisions(AnyCollidable auto& first, std::vector<std::shared_ptr<SCollidable>>& vec,
		bool checkIsActive = true, bool checkChannels = false) {
		for (auto& second : vec) {
			if (checkIsActive) {
				if (!first.bIsActive) return;
				if (!second->bIsActive) continue;
			}
			if (DetectCollision(first, *second)) {
				DispatchCollision(first, *second, checkChannels);
			}
		}
	}
	//I dont really like making it templated, but otherwise bools can not be defaulted (or can they?)
	template<bool checkIsActive = true, bool checkChannels = false, 
		AnyCollidable FCollidable, AnyCollidable...SCollidable>
	static void DispatchCollisions(AnyCollidable auto& first, std::vector<std::shared_ptr<FCollidable>>& vec, 
		std::vector<std::shared_ptr<SCollidable>>&...vecs) {
		if constexpr (sizeof...(SCollidable)) {
			for (auto& second : vec) {
				if (checkIsActive) {
					if (!first.bIsActive) return;
					if (!second->bIsActive) continue;
				}
				if (DetectCollision(first, *second)) {
					DispatchCollision(first, *second, checkChannels);
				}
			}
			DispatchCollisions<checkIsActive, checkChannels>(first, vecs...);
		}
		else {
			DispatchCollisions(first, vec, checkIsActive, checkChannels);
		}	
	}

	template<AnyCollidable FCollidable, AnyCollidable SCollidable>
	static void DispatchCollisions( std::vector<std::shared_ptr<FCollidable>>& first, 
		std::vector<std::shared_ptr<SCollidable>>& second, bool checkIsActive = true, bool checkChannels = false) {
		for (auto& fCollidable : first) {
			for (auto& sCollidable : second) {
				DetectDispatchCollision(*fCollidable, *sCollidable);
			}
		}
	}

protected:
	template<class FShape, class SShape>
	[[nodiscard]] static bool DetectCollision(const Collidable<FShape>& first, const Collidable<SShape>& second);
	static void DispatchCollision(CollidableBase& first, CollidableBase& second, bool checkChannels);
private:
	Collider() = delete;
};