#pragma once
#include "DragonJumpFramework.h"

//maybe should have gone with singleton after all
class Drawable
{
public:
	[[maybe_unused]] virtual bool DrawIfActive(float cameraVerticalOffset) 
		{ position.y += cameraVerticalOffset; return IsActive() ? DrawIfActive_Internal() : false; };
	[[nodiscard]] virtual bool IsActive() { return bIsActive; };
	virtual Vector2Df GetPosition() const { return position; };
	virtual void SetPosition(const Vector2Df& p)
		{ position = p; }
	virtual bool Reactivate(const Vector2Df& v) = 0;
	virtual void Deactivate() 
		{ bIsActive = false; };
protected:
	//Drawable(DragonJumpFramework& _framework) = delete;
	template<class...Args>
	Drawable(DragonJumpFramework& _framework, const Args&...args) : 
		framework{ _framework }, bIsActive{ true }, position{ args... } {};
	virtual bool DrawIfActive_Internal() = 0;
	DragonJumpFramework& framework;
	bool bIsActive;
	Vector2Df position;
};

