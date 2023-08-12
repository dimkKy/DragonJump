// by Dmitry Kolontay

#pragma once

#include "Drawable.h"

class Tickable : public virtual Drawable
{
public:
	//dispatcher is responsible for checking isActive
	virtual void ReceiveTick(float deltaTime) & = 0;
	[[nodiscard]] virtual Vector2Df GetVelocity() const 
		{ return velocity; };
	[[nodiscard]] virtual float GetMaxTickDeltaTime() const 
		{ return 0.f; };
	template<typename TTickable>
		requires std::is_base_of<Tickable, TTickable>::value
	static void DispatchTicks(float deltaTime, std::vector<std::shared_ptr<TTickable>>& vec) {
		for (auto& tickable : vec) {
			if (tickable->IsActive()) {
				tickable->ReceiveTick(deltaTime);
			}		
		}
	}

	template<typename FTickable, typename...STickable>
		requires std::is_base_of<Tickable, FTickable>::value &&
		((std::is_base_of<Tickable, STickable>::value) &&...)
		static void DispatchTicks(float deltaTime, std::vector<std::shared_ptr<FTickable>>& vec,
			std::vector<std::shared_ptr<STickable>>&...vecs) {
		for(auto & tickable : vec) {
			if (tickable->IsActive()) {
				tickable->ReceiveTick(deltaTime);
			}
		}
		DispatchTicks(deltaTime, vecs...);
	}
protected:
	template<class...Args>
	Tickable(const Args&...args) : 
		velocity{ args... } {};
	Vector2Df velocity;
};

