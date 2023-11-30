// by Dmitry Kolontay

#pragma once

#include "Drawable.h"
#include <span>
#include <type_traits>
#include <cassert>

enum class CollisionChannel {
	None,
	Platform,
	Monster,
	Pickup,
	Player,
	Projectile,
	COUNT,
};

namespace CC {
	inline constexpr CollisionChannel Platform{ CollisionChannel::Platform };
	inline constexpr CollisionChannel Monster{ CollisionChannel::Monster };
	inline constexpr CollisionChannel Pickup{ CollisionChannel::Pickup };
	inline constexpr CollisionChannel Player{ CollisionChannel::Player };
	inline constexpr CollisionChannel Projectile{ CollisionChannel::Projectile };
}

struct Shape {
protected:
	Shape() {};
};
struct RectangleShape : private Shape
{
	RectangleShape() : halfSize{} {};
	template<class...Args>
	RectangleShape(Args&&...args) : 
		halfSize{ std::forward<Args>(args)... } {
		assert(halfSize.x > 0.f && halfSize.y > 0.f, "size must be positive");
	};
	Vector2Df halfSize;
	explicit operator bool() const
		{ return halfSize.operator bool(); }
};
struct CircleShape : private Shape
{
	CircleShape() : radiusSquared{0.f} {};
	CircleShape(const float& f) : 
		radiusSquared{ f } 
	{
		assert(radiusSquared > 0.f, "radius must be positive");
	};
	float radiusSquared;
	explicit operator bool() const 
		{ return radiusSquared; }
};

// pivot for all collidables should be at (0.5, 0.5)
class CollidableBase : public virtual Drawable 
{
public:
	virtual void ReceiveCollision(CollidableBase& other) {};
	virtual CollisionChannel GetCollisionChannel() const = 0;
	virtual bool GetCollisionResponse(CollisionChannel channel) const = 0;
protected:
	CollidableBase() {};
};

template <class TShape>
	requires std::is_base_of_v<Shape, TShape> && 
		!std::is_same_v<Shape, TShape>
class Collidable : public CollidableBase
{
	friend class Collider;
public:
	//I use Collidable::IsActive() to check if 
	//collidable was inited properly aka has collision
	virtual bool IsActive() override 
		{ return bIsActive && collisionInfo; };
	TShape GetCollisionInfo() const 
		{ return collisionInfo; }
protected:
	template<class...Args>
	Collidable(Args&&...args):
		collisionInfo{ std::forward<Args>(args)... } {};
	TShape collisionInfo;
};

template <typename TDerived>
struct is_base_of_any_collidable
{
	template<typename TShape>
	static constexpr std::true_type is_collidable(const Collidable<TShape>&);
	static constexpr std::false_type is_collidable(...);
	using type = decltype(is_collidable(std::declval<TDerived&>()));
};

template <class TDerived, template<typename> typename TBase>
using is_base_of_any_collidable_t = is_base_of_any_collidable<TDerived, TBase>::type;

template <class TDerived, template<typename> typename TBase>
inline constexpr bool is_base_of_any_collidable_v{ is_base_of_any_collidable_t<TDerived, TBase>::value };

// no ckeck for TShape = Shape here
template<typename TDerived>
concept AnyCollidable = is_base_of_any_collidable_v<TDerived>;

extern template class Collidable<CircleShape>;
extern template class Collidable<RectangleShape>;