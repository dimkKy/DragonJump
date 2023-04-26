// by Dmitry Kolontay

#pragma once

#include "Clickable.h"
#include <functional>

//alignment - [0.5, 0.5]
class Button : public Clickable
{
public:
	Button(DragonJumpFramework& _framework, const Vector2Df& position, const std::string& spritePath);
		
	[[maybe_unused]] virtual bool DrawIfActive(float cameraVerticalOffset) override
		{ return IsActive() ? DrawIfActive_Internal() : false; };

	virtual bool Reactivate(const Vector2Df& pos) override ;

	virtual bool OnMouseButtonClick(const Vector2D& clickPos, bool bReleased)
		{ return bReleased ? OnMouseButtonUp(clickPos) : OnMouseButtonDown(clickPos); }
	virtual bool OnMouseButtonDown(const Vector2D& clickPos) override;
	virtual bool OnMouseButtonUp(const Vector2D& clickPos) override;
	
	std::function<void()> onClicked;
private:
	virtual bool DrawIfActive_Internal() override;
	bool bIsPressed;
	std::vector<SpriteInfo> sprites;
};

