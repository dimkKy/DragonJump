// by Dmitry Kolontay

#pragma once

#include "Drawable.h"
#include <functional>

//alignment - [1., 0.]
class IntDisplayer : public Drawable
{
public:
	IntDisplayer(DragonJumpFramework& _framework, const Vector2Df& position);
	[[maybe_unused]] virtual bool DrawIfActive(float cameraVerticalOffset) override
		{ return IsActive() ? DrawIfActive_Internal() : false; };
	virtual bool Reactivate(const Vector2Df& position) override;
	float GetWidgetHeight() const;

	std::function<int()> valueGetter;
protected:
	virtual bool DrawIfActive_Internal() override;
	
	std::vector<SpriteInfo> digits;
};

