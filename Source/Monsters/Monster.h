// by Dmitry Kolontay

#pragma once

#include "SteppableOn.h"

enum class MonsterType {
	Hole,
	Static,
	Movable,
	COUNT
};

namespace MT {
	inline constexpr PlatformType Hole{ MonsterType::Hole };
	inline constexpr PlatformType Static{ MonsterType::Static };
	inline constexpr PlatformType Movable{ MonsterType::Movable };
}

class MonsterBase {
public:
	virtual MonsterType GetMonsterType() const final 
		{ return type; };
protected:
	MonsterBase(MonsterType _type) : 
		type{ _type } {};
	MonsterType type;
};

template<class TShape>
class Monster : public SteppableOn<TShape>, public MonsterBase
{
public:
	virtual CollisionChannel GetCollisionChannel() const override 
		{ return CC::Monster; };

	virtual bool GetCollisionResponse(CollisionChannel channel) const override {
		return channel == CC::Projectile || 
			channel == CC::Player; 
	};
	
protected:
	Monster(MonsterType _type): 
		MonsterBase(_type) {};
};

