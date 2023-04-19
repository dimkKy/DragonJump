#pragma once

#include "Platform.h"
#include "Tickable.h"

class TickablePlatform : public Platform, public virtual Tickable
{
public:
	//virtual void OnJumpFrom(PlayerDoodle& other) override {};
	virtual bool SetPlatformType(PlatformType _type) = 0;
	virtual PlatformType GetPlatformType() const final { return type; };

	static float GetMaxTickDeltaTime() { return 0.5f; };
protected:
	TickablePlatform(DragonJumpFramework& _framework, PlatformType _type) :
		Platform(_framework), type{ _type } {};
	PlatformType type;
};

