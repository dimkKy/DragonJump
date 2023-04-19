#pragma once
#include "Drawable.h"
#include <span>
#include <type_traits>

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
	RectangleShape() {};
	template<class...Args>
	RectangleShape(Args...args) : size{ args } {};
	Vector2Df size;
	explicit operator bool() const
		{ return size.operator bool(); }
};
struct CircleShape : private Shape
{
	CircleShape() : radiusSquared{0.f} {};
	CircleShape(const float& _radiusSquared) : 
		radiusSquared{ _radiusSquared } {};
	CircleShape(const int& _radiusSquared) : 
		radiusSquared{ static_cast<float>(_radiusSquared) } {};
	float radiusSquared;
	explicit operator bool() const 
		{ return radiusSquared; }
};

/// <summary>
/// pivot for all collidables should be at (0.5, 0.5)
/// </summary>
class CollidableBase : public virtual Drawable 
{
public:
	virtual bool IsActive() = 0;
	virtual void ReceiveCollision(CollidableBase& other) = 0;
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
protected:
	template<class...Args>
	Collidable(const Args&...args):
		collisionInfo{ args... } {};
	TShape collisionInfo;
};

#include "Collidable.inl"