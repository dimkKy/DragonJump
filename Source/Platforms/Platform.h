// by Dmitry Kolontay

#pragma once

#include "SteppableOn.h"
#include <memory>

class Sprite;
class PlayerDoodle;

enum class PlatformType {
	Default,
	//
	OneOff,
	Weak,
	Trampoline,
	SelfDestuct,
	Invisible,
	//
	Vertical,
	PT_COUNT
};

namespace PT {
	inline constexpr PlatformType Default{ PlatformType::Default };
	inline constexpr PlatformType OneOff{ PlatformType::OneOff };
	inline constexpr PlatformType Weak{ PlatformType::Weak };
	inline constexpr PlatformType Trampoline{ PlatformType::Trampoline };
	inline constexpr PlatformType SelfDestuct{ PlatformType::SelfDestuct };
	inline constexpr PlatformType Invisible{ PlatformType::Invisible };
	inline constexpr PlatformType Vertical{ PlatformType::Vertical };
}

class Platform : public SteppableOn<RectangleShape>
{
public:
	Platform(DragonJumpFramework& _framework, const Vector2Df& pos);

	virtual bool IsActive() override;
	virtual bool Reactivate(const Vector2Df& position) & override;

	virtual CollisionChannel GetCollisionChannel() const override 
		{ return CC::Platform; };
	virtual bool GetCollisionResponse(CollisionChannel channel) const override 
		{ return channel == CC::Player; };
	
	virtual void OnJumpFrom(PlayerDoodle& other) override {};
protected:
	Platform(DragonJumpFramework& _framework) : 
		Drawable(_framework) {};

	[[maybe_unused]] virtual bool DrawIfActive_Internal() override;
	SpriteInfo defaultSprite;
};

