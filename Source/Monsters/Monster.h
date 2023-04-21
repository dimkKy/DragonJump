// by Dmitry Kolontay

#pragma once

#include "SteppableOn.h"

enum class MonsterType {
	MT_Hole,
	MT_Static,
	MT_Movable,
	MT_COUNT
};

class MonsterBase {
public:
	virtual MonsterType GetMonsterType() const final 
		{ return type; };
protected:
	//MonsterBase() = delete;
	MonsterBase(MonsterType _type) : 
		type{ _type } {};
	MonsterType type;
};

template<class TShape>
class Monster : public SteppableOn<TShape>, public MonsterBase
{
public:
	virtual CollisionChannel GetCollisionChannel() const override 
		{ return CollisionChannel::CC_Monster; };
	virtual bool GetCollisionResponse(CollisionChannel channel) const override {
		return channel == CollisionChannel::CC_Projectile || 
			channel == CollisionChannel::CC_Player; 
	};
	
protected:
	//Monster() = delete;
	Monster(MonsterType _type): 
		MonsterBase(_type) {};
};

