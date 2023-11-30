// by Dmitry Kolontay

#pragma once

#include "Platform.h"
#include "Tickable.h"

class TickablePlatform : public Platform, public Tickable
{
public:
	virtual PlatformType GetPlatformType() const final 
		{ return type; };

	static float GetMaxTickDeltaTime() noexcept { return 0.5f; };
protected:
	TickablePlatform(DragonJumpFramework& f, PlatformType t) :
		Platform(f), type{ t } {};
	PlatformType type;
};
