// by Dmitry Kolontay

#pragma once

#include "Drawable.h"

//aligniment [.0, .0]
class LifesDisplayer : public Drawable
{
public:
	LifesDisplayer(DragonJumpFramework& _framework, const Vector2Df& pos);
	[[maybe_unused]] virtual bool DrawIfActive(float cameraYOffset) override
		{ return IsActive() ? DrawIfActive_Internal() : false; };

	virtual bool Reactivate(const Vector2Df& pos) & override;

protected:
	virtual bool DrawIfActive_Internal() override;
	SpriteInfo sprite;
};

