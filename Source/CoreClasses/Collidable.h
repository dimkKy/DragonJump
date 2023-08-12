// by Dmitry Kolontay

#pragma once

#include "Drawable.h"
#include <span>
#include <type_traits>
#include <cassert>

enum class CollisionChannel {
	CC_None,
	CC_Platform,
	CC_Monster,
	CC_Pikup,
	CC_Player,
	CC_Projectile,
	CC_COUNT,
};

struct Shape {
protected:
	Shape() {};
};
struct RectangleShape : private Shape
{
	RectangleShape() : halfSize{} {};
	template<class...Args>
	RectangleShape(Args&...args) : 
		halfSize{ args } {
		assert(halfSize.x > 0.f && halfSize.y > 0.f && "size must be positive");
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
		assert(radiusSquared > 0.f && "radius must be positive");
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
	requires std::is_base_of<Shape, TShape>::value && 
		std::negation<std::is_same<Shape, TShape>>::value
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
	Collidable(const Args&...args):
		collisionInfo{ args... } {};
	TShape collisionInfo;
};

template <typename TDerived>
struct is_base_of_any_collidable
{
	template<typename TShape>
	static constexpr std::true_type is_collidable(const volatile Collidable<TShape>&);
	static constexpr std::false_type is_collidable(...);
	using value = decltype(is_collidable(std::declval<TDerived&>()));
};
// no ckeck for TShape = Shape here
template<typename TDerived>
concept AnyCollidable = is_base_of_any_collidable<TDerived>::value::value;

extern template class Collidable<CircleShape>;
extern template class Collidable<RectangleShape>;