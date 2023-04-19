#pragma once
#include "SteppableOn.h"
#include <memory>

class Sprite;
class PlayerDoodle;

enum class PlatformType {
	PT_Default,
	//
	PT_OneOff,
	PT_Weak,
	PT_Trampoline,
	PT_SelfDestuct,
	PT_Invisible,
	//
	PT_Vertical,
	PT_Horizontal,
	PT_Draggable,
	PT_COUNT
};

class Platform : public SteppableOn<RectangleShape>
{
public:
	Platform(DragonJumpFramework& _framework, const Vector2Df& pos);

	virtual bool IsActive() override;
	virtual bool Reactivate(const Vector2Df& position) override;

	virtual CollisionChannel GetCollisionChannel() const override { return CollisionChannel::CC_Platform; };
	virtual bool GetCollisionResponse(CollisionChannel channel) const override { return channel == CollisionChannel::CC_Player; };
	
	virtual void OnJumpFrom(PlayerDoodle& other) override {};
protected:
	Platform(DragonJumpFramework& _framework) : 
		Drawable(_framework) {};
	[[maybe_unused]] virtual bool DrawIfActive_Internal() override;
	SpriteInfo defaultSprite;
};

